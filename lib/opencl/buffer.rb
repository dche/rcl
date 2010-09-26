
module OpenCL
  
  # Buffer encaplulates the OpenCL memory object.
  #
  class Buffer
    # Buffer size in byte.
    attr_reader :byte_size
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
        @byte_size = size
      rescue Capi::CLError => e
        raise CLError.new(e.message)
      end
    end
    
    def initialize_copy(orig)
      begin
        @memory = @context.create_buffer(@io, @byte_size, nil)

        cq = @context.command_queue_of @context.default_device
        # CHECK: ensure all operations upon orig.memory are done?
        cq.enqueue_copy_buffer(orig.memory, @memory, 0, 0, self.byte_size, nil)
        cq.finish
      rescue Capi::CLError => e
        raise CLError.new(e.message)
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
        size = self.byte_size
      else
        size *= ts
      end
      if size < 1 || size > self.byte_size || not(Integer === size)
        raise ArgumentError, "Size must be larger than 0 and less than #{self.byte_size}." 
      end
      
      if self.byte_size < (offset + size) || pointer.byte_size < size
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
        raise CLError.new(e.message)
      end
                 
      self
    end
    
  end
  
end
