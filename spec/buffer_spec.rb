
require_relative 'spec_helper'

include OpenCL

describe Buffer do
  the 'Buffer::new' do
    buff = Buffer.new 1024
    buff.should.be.in
    buff.should.be.out
    buff.byte_size.should.equal 1024
    
    buff = Buffer.new 2048, :in
    buff.should.be.in
    buff.should.not.be.out
  end
  
  it 'should not refuse to write if pointer size is less then buffer size' do
    buff = Buffer.new 1024
    
    p = HostPointer.new :cl_uchar, 1023
    should.raise(ArgumentError) { buff.write p }
    should.raise(ArgumentError) { buff.read p }
  end
  
  it 'should reject if offset is not valid.' do
    buff = Buffer.new 4
    p = HostPointer.new :cl_uchar, 4
    
    should.raise(ArgumentError) { buff.write p, -1 }
    should.raise(ArgumentError) { buff.write p, 1.2 }
  end
  
  it 'should reject if size is invalid' do
    buff = Buffer.new 4
    p = HostPointer.new :cl_uchar, 2
    
    should.raise(ArgumentError) { buff.write p, 4, 1 }
    should.raise(ArgumentError) { buff.write p, 3, 2 }
    should.raise(ArgumentError) { buff.write p, 0, 3 }
  end
  
  the '#dup' do
    ptr = HostPointer.new :cl_float, 4
    ptr.assign [1, 2, 3, 4].pack('f4')
    
    buff = Buffer.new ptr.byte_size
    buff.write ptr
    
    clone = buff.dup
    clone.byte_size.should.equal buff.byte_size
    
    p = HostPointer.new :cl_float, 4
    clone.read p
    
    p[0].should.equal 1
    p[1].should.equal 2
    p[2].should.equal 3
    p[3].should.equal 4
  end
  
  the '#read and #write with offset' do
    ptr = HostPointer.new :cl_float, 8
    ptr.assign [1, 2, 3, 4].pack('f4')
    ptr[4].should.equal 0
    
    buff = Buffer.new ptr.byte_size
    buff.write ptr, 1, 4
    buff.read ptr, 2, 3
    
    ptr[0].should.equal 2
    ptr[1].should.equal 3
    ptr[2].should.equal 4
    ptr[3].should.equal 4
  end
  
  the '#map and #unmap' do
    ptr = HostPointer.new :cl_uint, 8
    buff = Buffer.new ptr.byte_size
    
    ptr.assign [1, 2, 3, 4, 5, 6, 7, 8]
    buff.write ptr
    
    should.not.raise(Exception) { buff.unmap_pointer }
    buff.pointer_mapped?.should.be.false
    
    mp = buff.map_pointer
    mp.should.is_a MappedPointer
    mp.byte_size.should.equal buff.byte_size
    mp.cast_to :cl_uint
    mp.type.should.equal :cl_uint

    mp[0].should.equal 1
    mp[1].should.equal 2
    mp[7].should.equal 8
    
    buff.pointer_mapped?.should.be.true
    
    mp[0] = 0xEEFF
    buff.unmap_pointer
    buff.read ptr
    ptr[0].should.equal 0xEEFF
    ptr[1].should.equal 2
    
    buff.pointer_mapped?.should.be.false
    mp.size.should.equal 0
    mp.null?.should.be.true
  end
  
end
