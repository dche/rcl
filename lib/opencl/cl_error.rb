# encoding: utf-8

module OpenCL
  # Errors occured when executing OpenCL functions.
  class CLError

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

  # ProgramBuildError conveys the build log.
  class ProgramBuildError < Exception; end
end
