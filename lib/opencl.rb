# encoding: utf-8
#
# copyright (c) 2010, Che Kenan (chekenan@gmail.com)

require File.join(File.dirname(__FILE__), 'opencl/capi_support')


# A rubyish interface to OpenCL.
module OpenCL
  class ProgramBuildError < Capi::CLError; end

  class CLError < Capi::CLError; end

  # Compiled code that can be executated on OpenCL devices.
  class Program
    # Capi::Context
    attr_reader :context
    # Capi::CommandQueue
    attr_reader :queue
    # The source code
    attr_reader :source

    def initialize(src, compile_options = '')
      begin
        devs = Capi.devices(Capi::CL_DEVICE_TYPE_GPU, nil)
        devs = Capi.devices(Capi::CL_DEVICE_TYPE_CPU, nil) if devs.empty?

        @context = Capi::Context.new nil, devs
        @device = devs.first
        @queue = @context.create_command_queue @device
        @kernels = {}
      rescue Capi::CLError => e
        raise CLError, e.message
      end
      self.compile src, compile_options
    end
    
    # 
    def compile(src, opts = '')
      return self if src == @source
      # TODO: protect this. Can't call #call when compile new source.
      begin
        @program = @context.create_program src
        @program.build [@device], opts, nil
      rescue Capi::CLError => e
        if e.message =~ /\(#{Capi::CL_BUILD_PROGRAM_FAILURE}\)/ # FIXME: ugly coupling.
          raise ProgramBuildError, @program.build_info(@device, Capi::CL_PROGRAM_BUILD_LOG)
        else
          raise CLError, e.message
        end
      end
      @kernels.clear
      @source = src
      self
    end
    
    # Executes a kernel.
    #
    # Kernel arguments is specified in a Hash in the form {value => type}
    def call(kernel, dim, size, args)      
      begin
        k = @kernels[kernel] || @kernels[kernel] = @program.create_kernel(kernel.to_s)
        
        if args.size != k.info(Capi::CL_KERNEL_NUM_ARGS)
          raise ArgumentError, "Wrong number of kernel arguments (#{args.size} for #{k.info(Capi::CL_KERNEL_NUM_ARGS)})."
        end

        args.each_with_index do |kv, idx|
          value, type = kv
          
          case type
          when :mem
            k.set_arg idx, value.memory
          else
            k.set_arg_with_type idx, type, value
          end
        end
        self.queue.enqueue_NDRange_kernel(k, dim, size, nil, nil)
        self.queue.finish
      rescue Capi::CLError => e
        raise CLError, e.message
      end
      self
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
      self.source
    end
  end
  
  # Buffer constains data on which a Program's kernels operate.
  # A Buffer object represents an OpenCL Memory object.
  #
  # A Buffer is always attached with a Program object. A Buffer created with
  # one Program can't be used in anohther program.
  #
  class Buffer
    # Buffer size
    attr_reader :size
    # Capi::Memory
    attr_reader :memory

    # Creates a buffer within the same context as of +program+.
    def initialize(program, size, io_flag = :in_out)
      @program = program
      @io = io_flag

      cl_io = Capi::CL_MEM_READ_WRITE
      case @io
      when :in_out
      when :in
        cl_io = Capi::CL_MEM_READ_ONLY
      when :out
        cl_io = Capi::CL_MEM_WRITE_ONLY
      else
        raise ArgumentError, 'Invalid memory in/out flag.'
      end

      begin
        @memory = program.context.create_buffer(cl_io, size, nil)
        @size = size
      rescue Capi::CLError => e
        raise CLError, e.message
      end
    end
    
    def method_missing(meth, *args, &blk)
      super
    end
    
    # Read from the device memory.
    def store_data_to(pointer)
      if self.out?
        begin
          @program.queue.enqueue_read_buffer(self.memory, true, 
                                             0, pointer.byte_size,
                                             pointer, nil);
          @program.queue.finish
        rescue Capi::CLError => e
          raise CLError, e.message
        end
      end
      self
    end
    alias :read :store_data_to
    
    # Write to the device memroy.
    def get_data_from(pointer)
      if self.in?
        begin
          @program.queue.enqueue_write_buffer(self.memory, true,
                                              0, pointer.byte_size,
                                              pointer, nil);
        rescue Capi::CLError => e
          raise CLError, e.message
        end
      end
      self
    end
    alias :write :get_data_from
    
    def in?
      @io != :out
    end
    
    def out?
      @io != :in
    end
  end
end
