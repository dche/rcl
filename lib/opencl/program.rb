
module OpenCL

  # Compiled code that can be executed on OpenCL devices.
  #
  #--
  # TODO: non-blocking, remove cq.finish!
  # TODO: a interface for compiler options?
  #++
  class Program

    # The source code
    attr_reader :source
    # Accumulated execution time of +call()+.
    # If profiling is disabled, returns 0.
    attr_accessor :execution_time

    #--
    # FIXME: disallow empty kernel string.
    #++
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
      rescue Capi::CLError => e
        cl_err = CLError.new e.message

        if cl_err.program_build_failed?
          raise ProgramBuildError, @program.build_info(@context.default_device, Capi::CL_PROGRAM_BUILD_LOG)
        else
          raise cl_err
        end
      ensure
        @mutex.unlock
      end
      self
    end

    # Returns the Capi::Kernel object with given +name+, or +nil+ if no
    # such a kernel in the receiver.
    #
    # Raises Capi::CLError if a kernel with given +name+ is not found in program.
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
    # local_size -- local work size, i.e., how many work-items in a workgroup.
    # args -- arguments of the kernel.
    def call(kernel_name, sizes = nil, *args)
      @mutex.lock
      begin
        k = self.kernel(kernel_name)
        # Ask the context for a device to execute the kernel.
        # The context object controls which device to use.
        device = @context.device

        gws = lws = 0

        if sizes.nil?
          raise ArgumentError, "No work sizes specified." unless block_given?

          max_lws = k.workgroup_size_on_device(device)
          lmem_size = device.local_memory_size
          gws, lws, args = yield max_lws, lmem_size
        else
          gws, lws = sizes
          if Integer === gws && lws.nil?
            gws = [gws]
          end
        end

        if args.size.odd? || args.size / 2 != k.argument_number
          raise ArgumentError, "Wrong number of kernel arguments, (#{args.size / 2} for #{k.argument_number})."
        end

        (args.size / 2).times do |i|
          type = args[i * 2]
          value = args[i * 2 + 1]

          case type
          when :mem
            k.set_arg i, type, value.memory
          else
            k.set_arg i, type, value
          end
        end
        
        # Get the work sizes. If anything goes wrong, the OpenCL runtime will
        # report it finally.
        # BUG
        gws, lws = sizes
        unless gws.is_a? Array
          gws = sizes
          lws = nil
        end
        
        cq = self.profiling? ? @context.profiling_command_queue_of(device) : @context.command_queue_of(device)   
        event = cq.enqueue_NDRange_kernel(k, gws.length, gws, lws, nil)
        cq.finish

        profiling event, kernel
      rescue Capi::CLError => e
        raise CLError.new(e.message)
      ensure
        @mutex.unlock
      end
      self
    end

    # Used by complex program (e.g., FFT) to ensure it can work on all devices.
    def max_workgroup_size
      kernels = @program.create_kernels

      @context.devices.map do |dev|
        kernels.map do |k|
          k.workgroup_size_on_device(dev)
        end.min
      end.min
    end

    def method_missing(meth, *args, &blk)
      begin
        self.call meth, args.first, *(args[1..-1])
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

    def profiling(event, kernel)
      if self.profiling?
        et = event.profiling_info(Capi::CL_PROFILING_COMMAND_END)
        st = event.profiling_info(Capi::CL_PROFILING_COMMAND_START)
        @execution_time = (et - st) * 1e-9
      end
    end

  end

end
