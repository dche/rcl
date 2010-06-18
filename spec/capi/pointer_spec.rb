
require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL

describe Pointer do
  
  before do
    @p = Pointer.new(:cl_int, 1024);
  end
  
  it "should reject invalid type." do
    should.raise(TypeError) { Pointer.new 'cl_int', 1024 }
    should.raise(ArgumentError) { Pointer.new :cl_int64, 1024 }
    should.raise(ArgumentError) { Pointer.new :float, 2 }
  end
  
  it "should reject 0 size" do
    should.raise(ArgumentError) { Pointer.new :cl_ulong, 0 }
    should.raise(ArgumentError) { Pointer.new :cl_ulong, 1.5 }
    should.not.raise(Exception) { Pointer.new :cl_ulong, 1 }
  end
  
  the 'Pointer address should be 128 bytes aligned.' do
    (Pointer.new(:cl_float4, 1).address & 0x7F).should.equal 0
    (@p.address & 0x7F).should.equal 0
  end
  
  the 'type()' do
    @p.type.should.equal :cl_int
    
    [:cl_char, :cl_uchar, :cl_short, :cl_ushort, :cl_int, :cl_uint, :cl_long,
     :cl_ulong, :cl_half, :cl_float].each do |t|
       Pointer.new(t, 1).type.should.equal t
     end
     
     ['char', 'short', 'int', 'long'].map do |t|
       ['cl_' + t, 'cl_u' + t]
     end.flatten.push('cl_float').map(&:to_sym).each do |t|
       Pointer.new(t, 123).type.should.equal t
     end
  end
  
  the 'size()' do
    @p.size.should.equal 1024
  end
  
  the 'free()' do
    p = Pointer.new :cl_char8, 256
    p.address.should.not.equal 0
    should.not.raise(RuntimeError) { p[0] }
    p.free
    should.raise(RuntimeError) { p[0] }
    p.address.should.equal nil
    p.should.be.null
    p.size.should.equal 0
    p.type.should.equal :cl_char8
  end
  
  it 'should reject access out of memory boundary.' do
    should.raise(RuntimeError) { @p[@p.size] }
    should.raise(RuntimeError) { @p[@p.size] = 1 }
  end
  
end
