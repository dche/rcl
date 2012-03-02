
require 'mutex_m'

module OpenCL

  # Compiled code that can be executed on OpenCL devices.
  #
  #--
  # TODO: a interface for compiler options? no?
  #++
  class Program

    # The source code
    attr_reader :source
    # Accumulated execution time of +call()+.
    # If profiling is disabled, returns 0.
    attr_accessor :execution_time

    def initialize(src = '', compile_options = '')
      @context = OpenCL::Context.default_context
      # Lock to prevent compiling while executing call()
      @mutex = Mutex.new

      @profiling = false
      @execution_time = 0

      @kernels = {}
      self.compile src, compile_options unless src.empty?
    end

    # Compile the program source.
    def compile(src, options = '')
      return self if src == @source

      @mutex.lock
      begin
        @program = @context.create_program src
        # Compilte the program on all available devices in the context.
        @program.build @context.devices, options, nil
        @source = src.freeze
      rescue CLError => e
        if e.program_build_failed?
          raise ProgramBuildError, @program.build_info(@context.default_device, CL_PROGRAM_BUILD_LOG)
        else
          raise e
        end
      ensure
        @mutex.unlock
      end
      self
    end

    # Returns the Capi::Kernel object with given +name+, or raises CLError
    # if a kernel with given +name+ is not found in program.
    def kernel(name)
      nm = name.to_s
      k = @kernels[nm]
      if k.nil?
        k = @program.create_kernel(nm)
        @kernels[nm] = k
      end
      return k
    end

    # Enable or disable profiling.
    #
    # yesno:: +true+ or +false+.
    def profiling=(yesno)
      @profiling = (yesno == true)
      self.execution_time = 0 unless self.profiling?
    end

    # Returns +true+ if profiling is enabled.
    def profiling?
      @profiling
    end

    # Executes a kernel.
    #
    # kernel_name -- the kernel name.
    # sizes -- global and local work sizes, can be nil if the work sizes are
    #          provided by calling back to the associated block.
    # args -- arguments of the kernel.
    def call(kernel_name, sizes = nil, *args)
      @mutex.lock
      begin
        k = self.kernel(kernel_name)
        # Ask the context for a device to execute the kernel.
        # The context object controls which device to use.
        device = @context.device

        gws = lws = 0

        # if sizes is nil, then we know the caller wants to query the program
        # object for max local workgroup size and local memory size.
        #
        # A block with arity 2 must be given to accept these values.
        if sizes.nil?
          raise ArgumentError, "no work sizes specified." unless block_given?

          max_lws = k.workgroup_size_on_device(device)
          lmem_size = device.local_memory_size
          gws, lws, args = yield max_lws, lmem_size
        else
          # sizes = [[dim1, dim2], [dim1, dim2]]
          gws, lws = sizes
          if Fixnum === gws
            gws = lws.nil? ? [gws] : sizes
          end
        end

        if args.size.odd? || args.size / 2 != k.argument_number
          raise ArgumentError, "wrong number of kernel arguments, (#{args.size / 2} for #{k.argument_number})."
        end

        (args.size / 2).times do |i|
          type = args[i * 2]
          value = args[i * 2 + 1]

          case type
          when :mem, :image2d, :image3d
            value.unmap_pointer
            k.set_arg i, type, value.memory
          else
            k.set_arg i, type, value
          end
        end

        cq = self.profiling? ? @context.profiling_command_queue_of(device) : @context.command_queue_of(device)
        # TOOD: Not wait for the execution to be completed? Find a case
        #       defeat it!
        profiling kernel_name, cq.enqueue_NDRange_kernel(k, gws.length, gws, lws, nil)
      rescue CLError => e
        raise e
      ensure
        @mutex.unlock
      end
      self
    end

    def method_missing(meth, *args, &blk)
      begin
        self.call meth, *args
      rescue CLError => e
        if e.invalid_kernel_name?
          super
        else
          raise e
        end
      end
    end

    def to_s
      "#<#{self.class} #{self.object_id}>"
    end

    private

    def profiling(kernel, event)
      if self.profiling?
        OpenCL::Capi.wait_for_events([event])
        et = event.profiling_info CL_PROFILING_COMMAND_END
        st = event.profiling_info CL_PROFILING_COMMAND_START
        @execution_time = (et - st) * 1e-9
      end
    end

  end

end
