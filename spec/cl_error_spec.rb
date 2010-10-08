require File.join(File.dirname(__FILE__), 'spec_helper')

include OpenCL

describe CLError do
  it 'should reject invalid error code.' do
    should.raise(ArgumentError) { CLError.new '0' }
    should.raise(ArgumentError) { CLError.new 'a12' }
CLError.new "#{Capi::CL_BUILD_PROGRAM_FAILURE}"
    should.not.raise(Exception) { CLError.new "#{Capi::CL_BUILD_PROGRAM_FAILURE}" }
    should.not.raise(Exception) { CLError.new '100_0000' }
  end

end
