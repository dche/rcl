# encoding: utf-8

module OpenCL
  # The Structure object stores value of structrued data type in Ruby side,
  # for transfering structured data to and from OpenCL buffers.
  #
  class Structure < BasicObject

    def initialize(type, pointer = nil)
      @type = type.is_a?(Type) ? type : Type.new(type)
      ::Kernel::raise ::ArgumentError, 'invalid data type, not a structure.' unless @type.structure?

      if pointer.nil?
        @pointer = HostPointer.new :cl_uchar, self.size
        wrap_address
      else
        self.wrap(pointer)
      end
    end

    def self.wrap(type, address)
      self.new type, address
    end

    attr_reader :type

    # Reuse the receiver, make it the accessor to another peice of buffer.
    def wrap(pointer)
      ::Kernel::raise ::ArgumentError, "" if pointer.byte_size < self.size
      @pointer = HostPointer.wrap_pointer pointer.address, :cl_uchar, self.size
      wrap_address
    end

    # Returns the size of the structure in byte.
    def size
      self.type.size
    end

    def method_missing(meth, *args)
      m = meth.to_s
      lv = (m =~ /(.*)=$/)
      m = $1 if lv

      type, ptr = @fields[m]
      return super if ptr.nil?

      if lv
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
      addr = @pointer.address
      tag = self.type.tag.clone

      until tag.empty?
        name, type, length = tag.shift(3)
        ptr = HostPointer.wrap_pointer addr, type.tag, length
        @fields[name] = [type, ptr]

        addr += ptr.byte_size
      end
    end
  end
end
