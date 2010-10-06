
module OpenCL

  class CLError < Exception
    
    def initialize(cl_err_msg)
      cl_err_msg = cl_err_msg.message unless cl_err_msg.is_a?(String)

      md = /^\((-?[1-9]([0-9]+)?)\)\s(\w[[:print:]]+)$/.match(cl_err_msg)
      raise ArgumentError, "Invalid CL error message." if md.nil?
      
      super md[3]
      @code = md[1].to_i
    end
    
    def program_build_failed?
      @code == Capi::CL_BUILD_PROGRAM_FAILURE
    end
    
    def invalid_kernel_name?
      @code == Capi::CL_INVALID_KERNEL_NAME
    end
    
    def inspect
      "#<#{self.class}: #{self.to_s}>"
    end
  end
  
  class ProgramBuildError < Exception; end

end
