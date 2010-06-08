
require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL::Capi

describe CLError do
  it "should not be instantiated by invalid error code." do
    should.raise(ArgumentError) { CLError.new(CL_SUCCESS) }   
  end
  
  it "should convey error code and message." do
    should.raise(CLError) { platforms.first.info(CL_SUCCESS) }

    begin
      platforms.first.info(CL_SUCCESS) # CL_SUCCESS must not be a valid param.
    rescue CLError => e
      e.cl_errcode.should.equal CL_INVALID_VALUE
    end
  end
end
