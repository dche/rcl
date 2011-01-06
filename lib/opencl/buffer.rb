# encoding: utf-8

module OpenCL

  # Buffer encaplulates the OpenCL memory object.
  #
  class Buffer
    # Buffer size in byte.
    attr_reader :byte_size
    # Capi::Memory object. You should not use it.
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
        @mapped_pointer = nil
      rescue Capi::CLError => e
        raise CLError.new(e)
      end
    end

    def initialize_copy(orig)
      begin
        @memory = @context.create_buffer(@io, @byte_size, nil)

        orig.unmap_pointer
        cq = @context.command_queue_of @context.default_device
        cq.enqueue_copy_buffer(orig.memory, @memory, 0, 0, self.byte_size, nil)

        @mapped_pointer = nil
      rescue Capi::CLError => e
        raise CLError.new(e.message)
      end
    end

    # Copy the contenst of the receiver to aother buffer.
    #
    # buffer - A +Buffer+ object. Its byte_size must be larger than
    #          the +size+.
    #
    def copy_to(buffer, size = nil, start = 0, target_start = 0)
      return self if buffer.equal? self

      size = self.byte_size if size.nil?
      return self if size <= 0

      start = 0 if start < 0
      target_start = 0 if target_start < 0

      if start + size > self.byte_size
        raise ArgumentError, "Specified buffer region (#{start}, #{start + size - 1}) is invalid."
      end

      if target_start + size > buffer.byte_size
        raise ArgumentError, "Specified buffer region (#{target_start}, #{target_start + size - 1}) is invalid."
      end

      self.unmap_pointer
      buffer.unmap_pointer

      begin
        mem_from = self.memory
        mem_to = buffer.memory

        cq = @context.command_queue_of @context.default_device
        cq.enqueue_copy_buffer(mem_from, mem_to, start, target_start, size, nil)
      rescue Capi::CLError => e
        raise CLError.new(e)
      end
      self
    end

    # Copy contents from another Buffer to the receiver.
    def copy_from(buffer, size = nil, start = 0, source_start = 0)
      buffer.copy_to self, size, source_start, start
      self
    end

    # Create a new Buffer, and copy +size+ bytes of the receiver
    # start from the offset +start+.
    def slice(start = 0, size = nil)
      size = self.byte_size if size.nil?
      return nil if size <= 0

      start = 0 if start < 0

      if (start + size) > self.byte_size
        raise ArgumentError, "Specified slice region (#{start}, #{start + size - 1}) is invalid."
      end

      io = (self.in? && self.out?) ? :in_out : (self.in? ? :in : :out)
      self.class.new(size, io).copy_from(self, size, 0, start)
    end

    # Read data from the device memory, and store the data to a HostPointer.
    #
    # Does nothing if the buffer's IO flag is :in, or the byte size of the
    # HostPointer is less than the size of memory.
    #
    # pointer:: A HostPointer object.
    #
    # offset::
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

    # Returns a MappedPointer object.
    #
    # Because the behavior that kernel executes on mapped Memory objects
    # is undefined, we restrict that there is only one
    #
    def map_pointer
      return @mapped_pointer unless @mapped_pointer.nil?

      begin
        cq = @context.command_queue_of @context.default_device
        @mapped_pointer, _ = cq.enqueue_map_buffer self.memory, true, map_flag, 0, self.byte_size, nil
      rescue Capi::CLError => e
        warn self.inspect + ".map(), " + CLError.new(e.message).to_s
        raise CLError.new(e.message)
      end
      return @mapped_pointer
    end

    # Un-maps the buffer object, or does nothing is the receiver is not mapped.
    #
    # Returns the receiver.
    def unmap_pointer
      return self if @mapped_pointer.nil?

      begin
        cq = @context.command_queue_of @context.default_device
        cq.enqueue_unmap_mem_object self.memory, @mapped_pointer, nil

        @mapped_pointer = nil
      rescue Capi::CLError => e
        warn self.inspect + ".unmap_pointer(), " + CLError.new(e.message).to_s
        raise CLError.new(e.message)
      end
      self
    end

    # Returns +true+ if the recevier has been mapped to a host pointer.
    def pointer_mapped?
      !@mapped_pointer.nil?
    end

    # Returns +true+ if the receiver is readable by the device.
    def in?
      @io != Capi::CL_MEM_WRITE_ONLY
    end

    # Returns +true+ ff the receiver is writable by the device.
    def out?
      @io != Capi::CL_MEM_READ_ONLY
    end

    def to_s
      io_str = self.in? ? "r" : ""
      io_str += "w" if self.out?

      sz = self.byte_size
      sz_str = if sz < 1024
        "#{sz} Bytes"
      elsif sz < 1024 * 1024
        "#{sz.fdiv(1024)} KB"
      else
        "#{sz.fdiv(1024 * 1024)} MB"
      end

      "#<#{self.class}: #{sz_str}, #{io_str}, #{@mapped_pointer ? 'mapped' : 'unmapped'}>"
    end

    private

    # Construct flag parameter for enqueue_map_buffer, based on the value
    # of @io.
    def map_flag
      flag = 0
      flag |= Capi::CL_MAP_READ if self.in?
      flag |= Capi::CL_MAP_WRITE if self.out?
      # The value of above statement is nil if self.out? is false.
      flag
    end

    # Do the work of read from/wrie to a HostPointer.
    def rw_mem(rw, pointer, offset, size)
      ts = pointer.type_size

      offset = 0 if offset < 0
      offset *= ts

      size = 0 if size < 0
      if size == 0
        size = self.byte_size
      else
        size *= ts
      end

      if size < 1 || size > self.byte_size
        raise ArgumentError, "Size must be larger than 0 and less than #{self.byte_size}."
      end

      if self.byte_size < (offset + size) || pointer.byte_size < size
        raise ArgumentError, "Size is too large."
      end

      begin
        # NOTE: Devices in same context share a combined memory bool.
        #       That's why we can always use the queue of default device,
        #       i.e., it's OK a buffer is written through a queue of one device,
        #       and read by the kernel executed on another device.
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
