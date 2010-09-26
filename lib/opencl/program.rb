
module OpenCL

  # Compiled code that can be executed on OpenCL devices.
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
    # kernel -- the kernel name.
    # sizes -- global work sizes and local work sizes.
    #          work sizes is specified in a multi-demension array.
    # args -- arguments of the kernel.
    def call(kernel, sizes, *args)
      @mutex.lock
      begin
        k = @program.create_kernel(kernel.to_s)        
        if args.size.odd? || args.size / 2 != k.argument_number
          raise ArgumentError, "Wrong number of kernel arguments, (#{args.size / 2} for #{k.argument_number})."
        end
        
        # Ask the context for a device to execute the kernel.
        # The context object controls which device to use.
        device = @context.device
        
        (args.size / 2).times do |i|
          type = args[i * 2]
          value = args[i * 2 + 1]
          
          case type
          when :mem
            k.set_arg i, value.memory
          else
            raise ArgumentError, "Invalid type #{type}." unless OpenCL.valid_type?(type)
            k.set_arg_with_type i, type, value
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

        profiling event
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
    
    def profiling(event)
      if self.profiling?
        et = event.profiling_info(Capi::CL_PROFILING_COMMAND_END)
        st = event.profiling_info(Capi::CL_PROFILING_COMMAND_START)
        @execution_time += (et - st) * 1e-9
      end
    end
    
  end

end
