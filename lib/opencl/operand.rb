
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
      @pointer = OpenCL::HostPointer.new type, length

      update_cl_program

      @length = length
      @type = type

      @dirty = @new = false
      self
    end
    
    def to_s
      "#<#{self.class}: @length = #{self.length}, @type = #{self.type}>"
    end
    
    private
    
    def new?
      @new
    end
    
    def dirty?
      @dirty
    end
    
    # Used in sub-class methods in which the CL computation changed
    # contents of device buffer. This method should be called to indicate
    # such situation.
    def need_sync   # :doc:
      @new = true; self
    end
    
    def not_new
      @new = false
    end
    
    def now_dirty
      @dirty = true
    end
    
    def not_dirty
      @dirty = false
    end
    alias :now_clean :not_dirty

    #--
    # FEATURE: support ruby's subscriber syntax? Like, v[-1], v[-2]
    #++
    def [](i)
      if new?
        self.store_data_to @pointer
        not_new
      end    
      @pointer[i]
    end
    
    def []=(i, x)
      @pointer[i] = x
      now_dirty
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
      if dirty?
        self.get_data_from @pointer
        now_clean
      end
    
      update_cl_program
      @@program.call meth, *args
      self
    end
    
  end
end
