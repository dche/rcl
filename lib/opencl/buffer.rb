# encoding: utf-8

module OpenCL
  # Buffer encaplulates the OpenCL memory object.
  #
  class Buffer
    # Buffer size in byte.
    attr_reader :byte_size
    # Capi::Memory object. Used by the Program object, and you should not use it.
    attr_reader :memory

    # Creates a Buffer object.
    def initialize(size, io_flag = :inout)
      @context = OpenCL::Context.default_context

      @io = case io_flag
      when :inout
        Capi::CL_MEM_READ_WRITE
      when :in
        @io = Capi::CL_MEM_READ_ONLY
      when :out
        @io = Capi::CL_MEM_WRITE_ONLY
      else
        raise ArgumentError, "invalid io flag. Expected :in, :out, :inout, got #{io_flag}"
      end

      @byte_size = size
      @memory = @context.create_buffer(@io, size, nil)
      @pinned_memory = @memory
      @pinned_pointer = nil
      @mapped_pointer = nil
    end

    def initialize_copy(orig)
      @memory = @context.create_buffer(@io, @byte_size, nil)

      orig.unmap_pointer
      cq = @context.command_queue_of @context.default_device
      cq.enqueue_copy_buffer(orig.memory, @memory, 0, 0, self.byte_size, nil)

      @pinned_memory = @memory
      @pinned_pointer = nil
      @mapped_pointer = nil
    end

    # Attaches a pinned memory object to the receiver.
    # This pinned memory object is allocated by OpenCL and always mapped to
    # a host pointer.
    #
    # Note: Concurrently write to a pinned memory may not work because the
    #       host memory is page-locked.
    def pin
      return self if self.pinned?

      self.unmap_pointer
      @pinned_memory = @context.create_buffer(@io | Capi::CL_MEM_ALLOC_HOST_PTR, @byte_size, nil)
      cq = @context.command_queue_of @context.default_device
      @pinned_pointer, _ = cq.enqueue_map_buffer @pinned_memory, true, map_flag, 0, self.byte_size, nil

      self
    end

    # Returns +true+ if there is pinned memory attached.
    def pinned?
      !@pinned_pointer.nil?
    end

    # Copies the content of the receiver to aother Buffer object.
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
        raise ArgumentError, "specified buffer region (#{start}, #{start + size - 1}) is invalid."
      end

      if target_start + size > buffer.byte_size
        raise ArgumentError, "specified buffer region (#{target_start}, #{target_start + size - 1}) is invalid."
      end

      self.unmap_pointer
      buffer.unmap_pointer

      mem_from = self.memory
      mem_to = buffer.memory

      cq = @context.command_queue_of @context.default_device
      cq.enqueue_copy_buffer(mem_from, mem_to, start, target_start, size, nil)

      self
    end

    # Copies contents from another Buffer to the receiver.
    def copy_from(buffer, size = nil, start = 0, source_start = 0)
      buffer.copy_to self, size, source_start, start
      self
    end

    # Increases the size of OpenCL memory object.
    def resize(sz)
      return self if sz <= self.byte_size

      self.unmap_pointer
      pinned = self.pinned?

      cq = @context.command_queue_of @context.default_device
      if pinned
        cq.enqueue_unmap_mem_object @pinned_memory, @pinned_pointer, nil
        @pinned_pointer = nil
      end
      mo = @context.create_buffer(@io, sz, nil)
      cq.enqueue_copy_buffer(@memory, mo, 0, 0, self.byte_size, nil)

      @memory = mo
      @pinned_memory = mo
      @pinned_pointer = nil
      @byte_size = sz

      self.pin if pinned
      self
    end

    # Creates a new Buffer, and copy +size+ bytes of the receiver
    # start from the offset +start+.
    def slice(start = 0, size = nil)
      size = self.byte_size if size.nil?
      return nil if size <= 0

      start = 0 if start < 0

      if (start + size) > self.byte_size
        raise ArgumentError, "specified slice region (#{start}, #{start + size - 1}) is invalid."
      end

      io = (self.in? && self.out?) ? :inout : (self.in? ? :in : :out)
      self.class.new(size, io).copy_from(self, size, 0, start)
    end

    # Reads data from the device memory, and stores the data to a HostPointer.
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

    # Writes the data pointed by a HostPointer to the device memroy.
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
    def map_pointer
      return @mapped_pointer if self.pointer_mapped?

      begin
        cq = @context.command_queue_of @context.default_device
        if self.pinned?
          cq.enqueue_read_buffer(self.memory, true, 0, self.byte_size, @pinned_pointer, nil);
          @pinned_pointer.clear_dirty
          @mapped_pointer = @pinned_pointer
        else
          @mapped_pointer, _ = cq.enqueue_map_buffer self.memory, true, map_flag, 0, self.byte_size, nil
        end
      rescue CLError => e
        warn self.inspect + ".map(), " + e.message
        raise e
      end
      return @mapped_pointer
    end

    # Un-maps the buffer object, or does nothing if the receiver is not mapped.
    #
    # Returns the receiver.
    #
    #--
    # FIXME: possible memory leak if the receiver is not unmapped.
    #++
    def unmap_pointer
      return self unless self.pointer_mapped?

      begin
        cq = @context.command_queue_of @context.default_device
        if self.pinned?
          if @pinned_pointer.dirty?
            cq.enqueue_write_buffer self.memory, true, 0, self.byte_size, @pinned_pointer, nil;
          end
        else
          cq.enqueue_unmap_mem_object self.memory, @mapped_pointer, nil
        end
        @mapped_pointer = nil
      rescue CLError => e
        warn self.inspect + ".unmap_pointer(), " + e.message
        raise e
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
      bsz = self.byte_size
      tsz = pointer.type_size

      offset = 0 if offset < 0
      offset *= tsz

      size = (size <= 0) ? bsz : size * tsz

      if size < 1 || size > bsz
        raise ArgumentError, "size must be larger than 0 and less than #{bsz}."
      end

      if bsz < (offset + size) || pointer.byte_size < size
        raise ArgumentError, "size is too large."
      end

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
      self
    end
  end
end
