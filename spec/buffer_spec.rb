
require File.join(File.dirname(__FILE__), 'spec_helper')

include OpenCL

describe Buffer do
  the 'Buffer::new' do
    buff = Buffer.new 1024
    buff.should.be.in
    buff.should.be.out
    buff.size.should.equal 1024
    
    buff = Buffer.new 2048, :in
    buff.should.be.in
    buff.should.not.be.out
  end
  
  it 'should not write to write only memory' do
    buff = Buffer.new 1024
    
    p = HostPointer.new :cl_uchar, 1023
    should.raise(ArgumentError) { buff.write p }
    should.raise(ArgumentError) { buff.read p }
  end
  
  the '#dup' do
  end
  
  the '#read and #write with offset' do
  end
  
end
