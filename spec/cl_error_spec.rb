
require File.join(File.dirname(__FILE__), 'spec_helper')

describe CLError do
  it 'should reject invalid error code.' do
    should.raise(ArgumentError) { CLError.new 0 }
    should.not.raise(Exception) { CLError.new "#{CL_BUILD_PROGRAM_FAILURE}" }
    should.not.raise(Exception) { CLError.new '100_0000' }
  end

  the 'message should not be a string of error code.' do
    ec = "#{CL_BUILD_PROGRAM_FAILURE}"
    e = CLError.new ec
    e.message.should.not.equal ec
  end
end
