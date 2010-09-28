
module OpenCL
  
  # The base class of Vector and Matrix
  class Operand < OpenCL::Buffer
    
    # The program to execute the computation on the Operand.
    @@program = nil
    # Kernel sources.
    @@kernel_sources = []
    
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
      map
      @pointer[i]
    end
    
    def []=(i, x)      
      map
      @pointer[i] = x

      self
    end
        
    #--
    # FIXME: should be a class method?
    #++
    def update_cl_program
      unless @@kernel_sources.empty?
        @@program = OpenCL::Program.new @@kernel_sources.reduce(:+), '-cl-mad-enable'
        @@kernel_sources.clear
      end
      raise 'No OpenCL kernel defined.' if @@program.nil?
    end
    
    # 
    def call_cl_method(meth, *args)   # :doc:
      update_cl_program
      unmap    
      # CHECK: prevent map when call_cl_method.
      @@program.call meth, *args
      self
    end
    
    def map
      return nil if mapped?
      super
      @pointer.cast_to self.type
    end
    
    private :map, :unmap, :mapped?
    
  end
end
