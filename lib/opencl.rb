
require File.join(File.dirname(__FILE__), 'opencl/capi_support')
require File.join(File.dirname(__FILE__), 'opencl/lib')

# A rubyish interface to OpenCL.
#
#--
# TODO: non-blocking
# TODO: a interface for compiler options?
#++
module OpenCL
  class ProgramBuildError < Capi::CLError; end

  class CLError < Capi::CLError; end
  
  class Context
    
    class <<self
      #--
      # Reduce the creation of OpenCL context and Device objects.
      #++
      def context_of(device_type)
        @contexts ||= {}
        
        dt = case device_type
        when :gpu
          Capi::CL_DEVICE_TYPE_GPU
        when :cpu
          dt = Capi::CL_DEVICE_TYPE_CPU
        else
          raise ArgumentError, 'Inalid device type.'
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
        raise CLError, e.message
      end
    end
    
    # Returns the default Capi::Device object.
    def default_device
      @context.devices.first
    end
    
    # Returns the a Capi::Device object based on certain dispatching mechanism.
    def device
      # TODO: choose device based on load
      self.default_device
    end
    
    # Max global memory the context can accept.
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
    
    def method_missing(meth, *args, &blk)
      case meth
      when :create_program, :devices, :create_buffer
        @context.send meth, *args, &blk
      else
        super
      end
    end
  end
    
  # Compiled code that can be executed on OpenCL devices.
  class Program
    # The source code
    attr_reader :source

    #--
    # FIXME: disallow empty kernel string.
    #++
    def initialize(src = '', compile_options = '')
      @context = OpenCL::Context.default_context
      @kernels = {}
      
      self.compile src, compile_options unless src.empty?
    end
    
    # Compile the program source.
    def compile(src, options = '')
      return self if src == @source
      # TODO: protect this. Can't call #call when compiling new source.
      begin
        @program = @context.create_program src
        # Compilte the program on all available devices in the context.
        @program.build @context.devices, options, nil
      rescue Capi::CLError => e
        if e.message =~ /\(#{Capi::CL_BUILD_PROGRAM_FAILURE}\)/
          # FIXME: ugly coupling above.
          #        try to raise a Exception object (not class) in capi.c
          raise ProgramBuildError, @program.build_info(@context.default_device, Capi::CL_PROGRAM_BUILD_LOG)
        else
          raise CLError, e.message
        end
      end
      @kernels.clear
      @source = src.freeze
      self
    end
    
    # Executes a kernel.
    #
    # kernel -- the kernel name.
    # sizes -- global work sizes and local work sizes.
    #          work sizes is specified in a multi-demension array.
    # args -- arguments of the kernel.
    def call(kernel, sizes, *args)      
      begin
        k = @kernels[kernel] || @kernels[kernel] = @program.create_kernel(kernel.to_s)
        
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
        gws, lws = sizes
        unless gws.is_a? Array
          gws = sizes
          lws = nil
        end
        
        cq = @context.command_queue_of device
        cq.enqueue_NDRange_kernel(k, gws.length, gws, lws, nil)
        cq.finish        
      rescue Capi::CLError => e
        raise CLError, e.message
      end
      self
    end
    
    # Used by complex program to ensure it can work on all devices.
    def max_workgroup_size
      kernels = @program.create_kernels
      
      @context.devices.map do |dev|
        kernels.map do |k|
          k.workgroup_size_on_device(dev)
        end.min
      end.min
    end
    
    def method_missing(meth, *args, &blk)
      if @kernels.has_key?(meth)
        self.call(meth, *args) 
      else
        begin
          k = @program.create_kernel(meth.to_s)
          @kernels[meth] = k
          self.call(meth, *args)
        rescue Capi::CLError => e
          super
        end
      end
    end
    
    def to_s
      "#<#{self.class} #{self.object_id}>"
    end
    
  end
  
  # Buffer encaplulates the OpenCL memory object.
  #
  class Buffer
    # Buffer size in byte.
    attr_reader :size
    # Capi::Memory. You should not use it.
    attr_reader :memory

    # Creates a Buffer object.
    def initialize(size, io_flag = :in_out)
      @context = OpenCL::Context.default_context
      
      @io = case io_flag
      when :in_out
        Capi::CL_MEM_READ_WRITE
      when :in
        @io = Capi::CL_MEM_READ_ONLY
      when :out
        @io = Capi::CL_MEM_WRITE_ONLY
      else
        raise ArgumentError, "Invalid io flag. Expected :in, :out, :in_out, got #{io_flag}"
      end

      begin
        @memory = @context.create_buffer(@io, size, nil)
        @size = size
      rescue Capi::CLError => e
        raise CLError, e.message
      end
    end
    
    def initialize_copy(orig)
      begin
        @memory = @context.create_buffer(@io, @size, nil)

        cq = @context.command_queue_of @context.default_device
        # CHECK: ensure all operations upon orig.memory are done?
        cq.enqueue_copy_buffer(orig.memory, @memory, 0, 0, self.size, nil)
        cq.finish
      rescue Capi::CLError => e
        raise CLError, e.message
      end
    end
    
    # Read data from the device memory, and store the data to a HostPointer.
    #
    # Does nothing if the buffer's io flag is :in, or the byte size of the
    # HostPointer is less than the size of memory.
    #
    # (HostPointer) pointer - 
    #
    # (Integer) offset - 
    #
    # Returns the receiver.
    def store_data_to(pointer, offset = 0, size = 0)
      return self unless self.out?
      rw_mem :read, pointer, offset, size
    end
    alias :read :store_data_to
    
    # Write the data pointed by a HostPointer to the device memroy.
    #
    # (HostPointer) pointer -
    # (Integer) offset - 
    #
    # Returns the receiver.
    def get_data_from(pointer, offset = 0, size = 0)
      return self unless self.in?
      rw_mem :write, pointer, offset, size
    end
    alias :write :get_data_from
    
    # Returns +true+ if the receiver is readable by the device.
    def in?
      @io != Capi::CL_MEM_WRITE_ONLY
    end
    
    # Returns +true+ ff the receiver is writable by the device.
    def out?
      @io != Capi::CL_MEM_READ_ONLY
    end
    
    def inspect
      "<#{self.class} #{self.object_id}>"
    end
    
    private
    
    def rw_mem(rw, pointer, offset, size)
      ts = pointer.type_size
      
      offset *= ts
      if offset < 0 || not(Integer === offset)
        raise ArgumentError, 'Offset must be 0 or a positive Integer.' 
      end
           
      if size == 0
        size = self.size
      else
        size *= ts
      end
      if size < 1 || size > self.size || not(Integer === size)
        raise ArgumentError, "Size must be larger than 0 and less than #{self.size}." 
      end
      
      if self.size < (offset + size) || pointer.byte_size < size
        raise ArgumentError, 
              "Size is too large." 
      end
      
      begin
        # NOTE: Devices in same context share a combined memory bool.
        #       That's why we can write buffer through a queue of one device,
        #       and read through queue of another device.
        cq = @context.command_queue_of @context.default_device
        if rw == :read
          cq.enqueue_read_buffer(self.memory, true, offset, size, pointer, nil);
        else
          cq.enqueue_write_buffer(self.memory, true, offset, size, pointer, nil);
        end        
      rescue Capi::CLError => e
        raise CLError, e.message
      end
                 
      self
    end
    
  end
  
  module_function
  
  def type_size(type)
    SCALAR_TYPES[type] || VECTOR_TYPES[type]
  end
  
  def valid_type?(type)
    SCALAR_TYPES.has_key?(type) || VECTOR_TYPES.has_key?(type)
  end
end
