
module OpenCL

  class Context

    class <<self
      #--
      # Reduce the number of creation of OpenCL context and Device objects.
      #++
      def context_of(device_type)
        @contexts ||= {}

        dt = case device_type
        when :gpu
          Capi::CL_DEVICE_TYPE_GPU
        when :cpu
          dt = Capi::CL_DEVICE_TYPE_CPU
        else
          raise ArgumentError, 'Invalid device type.'
        end
        @contexts[device_type] ||= new(dt)
      end

      def default_context
        context_of(:gpu) || context_of(:cpu)
      end

      private :new
    end

    def initialize(device_type)
      begin
        devs = Capi.devices(device_type, nil)
        raise RuntimeError, "No deivce of given type found." if devs.empty?

        @context = Capi::Context.new nil, devs
        @queues = {}

        devs.each do |dev|
          @queues[dev] = @context.create_command_queue dev
        end
        @queues.freeze
      rescue Capi::CLError => e
        raise CLError.new(e.message)
      end
    end

    # Returns the default Capi::Device object.
    def default_device
      @context.devices.first
    end

    # Returns the a Capi::Device object based on certain dispatching mechanism.
    def device
      # TODO: choose device based on load. How? Possible?
      self.default_device
    end

    # Maximum global memory the context can allocate.
    def max_mem_alloc_size
      @mgm ||= @context.devices.map do |dev|
        dev.max_mem_alloc_size
      end.min
    end

    # Returns the Capi::CommandQueue object of given device.
    #
    # Returns +nil+ if the +device+ is invalid.
    def command_queue_of(device)
      @queues[device]
    end

    # Returns a profiling enabled Capi::CommandQueue object of given device.
    def profiling_command_queue_of(device)
      @context.create_profiling_command_queue(device)
    end

    def method_missing(meth, *args, &blk)
      case meth
      when :create_program, :devices, :create_buffer
        @context.send meth, *args, &blk
      else
        super
      end
    end
  end

end
