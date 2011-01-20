# encoding: utf-8

module OpenCL

  # An Operand object is a typed Buffer that has a program associated with it.
  class Operand < Buffer

    @@common_source = File.read File.join(File.dirname(__FILE__), 'program.cl')
    @@libraries = []
    @@programs = {}

    # Makes a Library to be usable by a class of Operand objects.
    #
    # Note
    #
    # if an Operand object (instance of subclasses of Operand,
    # e.g., Vector, Matrix, and Set) uses a Library is depends on the type
    # of the object.
    #
    def self.use(lib)
      return if @@libraries.include?(lib)
      raise ArgumentError, "expected a OpenCL::Library" unless lib < Library
      @@libraries << lib
      nil
    end

    # Create a new Operand (a Buffer).
    def initialize(size, type)
      @size = size.to_i
      raise ArgumentError, 'expected size is larger than 0' if @size < 1
      @type = Type.new(type)

      @library_version = 0
      @program_source = @@common_source + self.type.to_cdef
      update_library

      bsz = size * @type.size
      super bsz
    end

    # Data type of elements.
    attr_reader :type

    # Number of items.
    attr_reader :size
    alias :length :size

    def program_source
      update_library if library_outdated?
      @program_source.clone
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
        @mapped_pointer.cast_to(self.type.tag) unless self.type.structure?
      end
      @mapped_pointer
    end

    def [](i)
      map_pointer[i]
    end

    def []=(i, v)
      map_pointer[i] = v
      v
    end

    # Recursively executes a reduction kernel.
    def reduce(kernel, n = self.length)
      # create a hidden buffer, the size is at most a half of self.size.
      if @reduction_buffer.nil?
        sz = next_gws(self.length)
        @reduction_buffer = self.class.new sz, self.type.tag
      end
      out = @reduction_buffer
      out[0] = self[0] if self.length == 1

      if n == 1
        res = out[0]
        out.unmap_pointer
        return res
      end

      groups = 1
      ts = self.type.size

      in_buff = (n == self.length) ? self : out
      execute_kernel(kernel) do |max_workgroup_size, lmem_size|
        gws = next_gws(n)
        lws = [max_workgroup_size, lmem_size / (2 * ts), gws].min

        # If n is small enough (< 256), one workgroup can reduce all items.
        #--
        # NOTE: Here we limit the workgroup number can't exceed 64.
        #       The effect is that the depth of the recursion will
        #       not larger than 3.  However, this limitation also
        #       results that there are no more than 512 * 64 = 32768
        #       work items. For very large n, this means each work
        #       item will do much work.
        #       Do some performance test on this limitation?
        #       Try 128, 256, or more?
        #++
        groups = gws < 64 ? 1 : [gws / lws, 64].min
        gws = groups * lws
        [[gws], [lws], [:mem, in_buff, :mem, out, :local, (lws * ts), :cl_uint, (next_pow2(n) / gws).ceil, :cl_uint, n]]
      end
      # After execute_kernel, the first #<groups> elements contains the
      # reduced values for next pass of reduction.
      return reduce(kernel, groups)
    end

    # Execute a map kernel.
    def map(kernel_name, out = nil)
      unless out.nil?
        verify_output out
      else
        out = self
      end
      execute_kernel kernel_name, [self.length], :mem, self, :cl_uint, self.length, :mem, out
    end

    private

    def verify_output(out)
      raise ArgumentError, "expected an Operand" unless out.is_a?(Operand)
      raise ArgumentError, "the output Operand has incompatible type." unless self.type_compatible?(out)
    end

    # Build the OpenCL Program for receiver's type.
    def program
      tag = self.type.tag
      if (@@programs[tag].nil? || library_outdated?)
        update_library
        src = ERB.new(@program_source).result(binding)
        @@programs[tag] = Program.new src, '-cl-mad-enable'
      end
      @@programs[tag]
    end

    # Execute a kernel.
    def execute_kernel(kernel_name, *args, &blk)
      unmap_pointer
      program.call kernel_name, *args, &blk
      self
    end

    def library_outdated?
      @library_version < @@libraries.length
    end

    def update_library
      return nil unless library_outdated?

      @library_version.upto(@@libraries.length - 1) do |i|
        lib = @@libraries[i]
        next unless self.type.compatible?(lib.type)

        mod = lib.interface_module
        # the library does not provide any ruby method to call, so it's useless.
        next if mod.nil?

        @program_source << lib.source.to_s
        self.extend mod
      end
      @library_version = @@libraries.length
      nil
    end

    def next_gws(n)
      np2 = next_pow2(n)
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
