# encoding: utf-8

module OpenCL
  # The Structure object stores value of structrued data type in Ruby side,
  # for transfering structured data to and from OpenCL buffers.
  #
  class Structure < BasicObject

    def initialize(type, pointer = nil, offset = 0)
      @type = type.is_a?(Type) ? type : Type.new(type)
      ::Kernel::raise ::ArgumentError, 'invalid data type, not a structure.' unless @type.structure?

      @fields = {}
      if pointer.nil?
        @pointer = HostPointer.new :cl_uchar, self.size
        @address = @pointer.address
        wrap_address
      else
        self.wrap(pointer, offset)
      end
    end

    # Same as calling +Structure.new(type, address, offset)+
    def self.wrap(type, address, offset = 0)
      self.new type, address, offset
    end

    attr_reader :type

    def pointer
      @address
    end

    # Reuse the receiver, make it the accessor to another piece of memory.
    def wrap(pointer, offset = 0)
      if offset < 0 || pointer.byte_size - offset * self.size < self.size
        ::Kernel::raise ::ArgumentError, "pointer size is too small."
      end
      @pointer = pointer
      @address = @pointer.address + offset * self.size
      wrap_address
    end

    # Returns the size of the structure in byte.
    def size
      @type.size
    end

    def respond_to?(meth)
      m = meth.to_s
      m = $1 if (m =~ /(.*)=$/)
      @fields.has_key?(meth)
    end

    def method_missing(meth, *args)
      m = meth.to_s
      lval = (m =~ /(.*)=$/)
      m = $1 if lval

      type, ptr = @fields[m]
      return super if ptr.nil?

      if lval
        narg = args.length
        val = args
        if args.length == 1 && args[0].is_a?(::String)
          val = args[0]
          narg = val.bytesize
        end
        if narg != ptr.size
          ::Kernel::raise ::ArgumentError, "wrong number of arguments (#{narg} for #{ptr.size})"
        end
        ptr.assign val
        @pointer.mark_dirty if MappedPointer === @pointer
      else
        if ptr.size > 1
          ptr.to_a
        else
          ptr[0]
        end
      end
    end

    def inspect
      "#<#{self.class}:#{self.object_id}, Structure: #{self.type}>"
    end

    private

    # Interpretes a piece of buffer, creates access pointers for each field.
    def wrap_address
      @fields = {}
      addr = @address
      tag, _ = @type.parse_structure_tag(@type.tag)

      until tag.empty?
        name, type, length = tag.shift(3)
        ptr = HostPointer.wrap_pointer addr, type.tag, length
        @fields[name] = [type, ptr]

        addr += ptr.byte_size
      end
    end
  end
end
