require_relative 'spec_helper'

include OpenCL

describe CLError do
  it 'should reject invalid error message.' do
    should.raise(ArgumentError) { CLError.new '(012) This is not a valid message.' }
    should.raise(ArgumentError) { CLError.new '(12)  This is not a valid message.' }
    should.raise(ArgumentError) { CLError.new '(a12) This is not a valid message.' }
    should.not.raise(Exception) { CLError.new '(12) This is a valid message.' }
    
  end
  
  it 'should strip the error code from error message.' do
    err = CLError.new '(12) This is a valid message.'
    err.message.should.equal 'This is a valid message.'
  end
end
