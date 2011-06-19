# encoding: utf-8

module OpenCL
  # Errors occured when executing OpenCL functions.
  class CLError < RuntimeError
    # Creates a CLError object.
    #--
    # called in the C extension.
    #++
    def initialize(cl_err_msg)
      cl_err_msg = cl_err_msg.message unless cl_err_msg.is_a?(String)

      code = cl_err_msg.to_i
      raise ArgumentError, "invalid CL error code." if code == 0

      @code = code
      msg = Capi::ERROR_MESSAGES[code] || "Unrecognized CLError."
      super msg
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
