# encoding: utf-8

module OpenCL
  # An Operand object is a typed Buffer that has a program associated with it.
  class Operand < Buffer

    class << self
      def inherited(cls)
        @libraries ||= []
        @libraries.each do |lib|
          cls.use lib
        end
      end

      # Makes a Library to be usable by a class of Operand objects.
      #
      # Note
      #
      # if an Operand object (instance of subclasses of Operand,
      # e.g., Vector, Matrix, and Set) uses a Library is depends on the type
      # of the object.
      #
      def use(lib)
        @libraries ||= []
        return if @libraries.include?(lib)
        raise ArgumentError, "expected a OpenCL::Library" unless lib < Library
        @libraries << lib
        nil
      end

      # Returns the OpenCL Program object for give type +type+.
      #
      def program(type)
        @programs ||= {}

        tag = type.tag
        version = self.libraries.length

        self.cl_source(type)
        src, ver = @program_sources[tag]

        if @programs[tag].nil? || ver < version
          @programs[tag] = Program.new src, '-cl-mad-enable'
        end
        @programs[tag]
      end

      # Returns the CL source code for given data type +type+.
      def cl_source(type)
        @program_sources ||= {}

        libs = self.libraries
        version = libs.length
        tag = type.tag

        src, ver = @program_sources[tag]
        if src.nil? || ver < version
          if src.nil?
            ver = 0
            src = type.to_cdef
          end
          # collect program sources.
          ver.upto(version - 1) do |i|
            lib = libs[i]
            next unless type.compatible?(lib.type)
            src << lib.source.to_s
          end
          cl_src = ERB.new(src).result(binding)
          @program_sources[tag] = [cl_src, version]
        end
        @program_sources[tag].first
      end

      # Returns the Array of Libraries used by the receiver.
      def libraries
        @libraries ||= []
      end
    end

    # Create a new Operand object.
    def initialize(size, type)
      @size = size.to_i
      raise ArgumentError, 'expected size is larger than 0' if @size < 1
      @type = Type.new(type)

      @library_version = 0
      update_library

      byte_size = size * @type.size
      super byte_size
    end

    # Data type of elements.
    attr_reader :type

    # Number of items.
    attr_reader :size
    alias :length :size

    def initialize_copy(from)
      super
      @reduction_buffer = nil
    end

    def resize(sz)
      return self if sz <= self.size

      bsz = sz * self.type.size
      super(bsz)

      @reduction_buffer = nil
      @size = sz
    end

    def respond_to?(meth)
      update_library if library_outdated?
      super
    end

    def method_missing(meth, *args, &blk)
      return super unless library_outdated?
      update_library
      # re-send the message.
      self.send meth, *args, &blk
    end

    def inspect
      "#{self.class}[#{self.length} of #{self.type}]"
    end

    def map_pointer
      unless pointer_mapped?
        super
        tag = self.type.structure? ? :cl_uchar : self.type.tag
        @mapped_pointer.cast_to(tag)
      end
      @mapped_pointer
    end

    # Returns the ith element in the OpenCL buffer.
    #
    def [](i)
      self.map_pointer
      return @mapped_pointer[i] unless self.type.structure?
      Structure.new self.type, @mapped_pointer, i
    end

    def []=(i, v)
      self.map_pointer
      unless self.type.structure?
        @mapped_pointer[i] = v
      else
        if !v.is_a?(Structure) || v.type != self.type
          raise TypeError, 'type mismatch.'
        end
        sz = self.type.size
        @mapped_pointer.assign_pointer v.pointer, sz, i * sz
      end
      v
    end

    # Recursively executes a reduction kernel.
    def reduce(kernel, n = self.length)
      return self[0] if self.length == 1

      # create a hidden buffer, the size is a half of self.size at most.
      if @reduction_buffer.nil?
        sz = next_gws(self.length)
        @reduction_buffer = Operand.new sz, self.type.tag
      end
      out = @reduction_buffer
      return out[0] if n == 1

      groups = 1
      ts = self.type.size

      in_buff = (n == self.length) ? self : out
      execute_kernel(kernel) do |max_workgroup_size, lmem_size|
        gws = next_gws(n)
        lws = [max_workgroup_size, lmem_size / (2 * ts), gws].min

        # NOTE: Here we limit the workgroup number can't exceed 64.
        #       The effect is that the depth of the recursion will
        #       not larger than 3 (n -> 64 -> 1).  This limitation
        #       also results that there are no more than 512 * 64 = 32768
        #       work items. For very large n, this means each work
        #       item will do much work.
        #       Do some performance test on this limitation?
        #       Try 128, 256, or more?
        groups = gws < 64 ? 1 : [gws / lws, 64].min
        gws = groups * lws
        [[gws], [lws], [:mem, in_buff, :mem, out, :local, (lws * ts), :cl_int, (next_pow2(n) / gws).ceil, :cl_int, n]]
      end
      # after execute_kernel, the first #<groups> elements of +out+ contains the
      # reduced values for next pass of reduction. The recursive terminates
      # when <groups> is 1.
      return reduce(kernel, groups)
    end

    # Execute a map kernel.
    def map(kernel_name, out = nil)
      unless out.nil?
        raise ArgumentError, "expected an Operand" unless out.is_a?(Operand)
        raise ArgumentError, "the output Operand has incompatible type." unless self.type_compatible?(out)
      else
        out = self
      end
      execute_kernel kernel_name, [self.length], :mem, self, :cl_int, self.length, :mem, out
    end

    private

    def program
      self.class.program(self.type)
    end

    # Execute a kernel.
    def execute_kernel(kernel_name, *args, &blk)
      program.call kernel_name, *args, &blk
      self
    end

    def library_outdated?
      @library_version < self.class.libraries.length
    end

    def update_library
      return nil unless library_outdated?

      libs = self.class.libraries
      @library_version.upto(libs.length - 1) do |i|
        lib = libs[i]
        next unless self.type.compatible?(lib.type)
        mod = lib.interface_module
        # the library does not provide any ruby method to call, so it's useless.
        next if mod.nil?
        self.extend mod
      end
      @library_version = libs.length
      nil
    end

    def next_gws(n)
      np2 = next_pow2(n)
      # If n is small enough (< 256), one workgroup can reduce all items.
      np2 < 256 ? 1 : np2 / 4
    end

    def next_pow2(n)
      n -= 1
      n |= n >> 1
      n |= n >> 2
      n |= n >> 4
      n |= n >> 8
      n |= n >> 16
      n + 1
    end
  end
end
