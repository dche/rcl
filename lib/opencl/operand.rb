
require File.join(File.dirname(__FILE__), 'operand/string')

module OpenCL

  # The base class of Vector and Matrix
  class Operand < OpenCL::Buffer

    # The program to execute the computation on the Operand.
    @@program = nil
    # Kernel sources.
    @@kernel_sources = []

    @@cl_modules = []
    @@cl_module_sources = ''

    def self.use_cl_modules(*modules)
      modules.reverse_each do |mod|
        next if @@cl_modules.member?(mod)
        @@cl_modules.unshift mod
        @@cl_module_sources.rcl_include(mod.to_s)
      end
    end

    def self.cl_modules_in_use
      @@cl_modules.dup
    end

    def self.add_kernels(string)
      raise ArgumentError, "Invalid CL source code." if !string.is_a?(String) || string.empty?
      @@kernel_sources << string    
    end

    # Number of items.
    attr_reader :length
    alias :size :length

    # Type of items.
    attr_reader :type

    def initialize(length, type = :cl_float)

      super OpenCL.type_size(type) * length

      # Collects kernel sources and build @@program.
      update_cl_program

      @length = length
      @type = type

      self
    end

    def to_s
      "#<#{self.class}: @length = #{self.length}, @type = #{self.type}>"
    end

    private

    def [](i)
      map_pointer[i]
    end

    def []=(i, x)
      map_pointer[i] = x
      self
    end

    def map_pointer
      unless pointer_mapped?
        super
        @mapped_pointer.cast_to self.type
      end
      @mapped_pointer
    end

    private :map_pointer, :unmap_pointer, :pointer_mapped?

    private

    def update_cl_program
      unless @@kernel_sources.empty?
        src = @@cl_module_sources + @@kernel_sources.reduce(:+)
        @@program = OpenCL::Program.new src, '-cl-mad-enable'
        @@kernel_sources.clear
      end
      raise 'No OpenCL kernel defined.' if @@program.nil?
    end

    #
    def call_cl_method(meth, *args)   # :doc:
      update_cl_program
      unmap_pointer
      # CHECK: prevent map when call_cl_method.
      @@program.call meth, *args
      self
    end

  end
end

require File.join(File.dirname(__FILE__), 'operand/reduce')
require File.join(File.dirname(__FILE__), 'operand/sort')

