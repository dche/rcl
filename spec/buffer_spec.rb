
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

  the '#copy_to' do
    mp = Buffer.new 1024 * OpenCL.type_size(:cl_uint), :out
    ptr = mp.map_pointer
    ptr.cast_to :cl_uint

    ptr.assign [1, 2, 3, 3, 2, 0xEEFF]

    mp_c = Buffer.new mp.byte_size, :in
    ptrc = mp_c.map_pointer
    ptrc[0].should.equal 0
    ptrc[1].should.equal 0
    ptrc[5].should.equal 0

    should.raise(ArgumentError) {
      mp.copy_to mp_c, mp.byte_size + 1
    }
    should.raise(ArgumentError) {
      mp.copy_to mp_c, mp.byte_size, 1
    }
    should.raise(ArgumentError) {
      mp.copy_to mp_c, mp.byte_size, 0, 1
    }
    should.not.raise(ArgumentError) {
      mp.copy_to mp_c, -1
      mp.copy_to mp_c, 0
      mp.copy_to mp_c, nil, -1, -2
    }

    mp_c.should.not.be.pointer_mapped
    mp.should.not.be.pointer_mapped
    mp.copy_to mp_c

    ptrc = mp_c.map_pointer
    ptrc.cast_to :cl_uint
    ptrc[0].should.equal 1
    ptrc[1].should.equal 2
    ptrc[5].should.equal 0xEEFF
  end

  the '#copy_from' do
    mp = Buffer.new 1024 * OpenCL.type_size(:cl_uint), :out
    ptr = mp.map_pointer
    ptr.cast_to :cl_uint
    ptr.assign [1, 2, 3, 3, 2, 0xEEFF]

    mp_c = Buffer.new mp.byte_size, :in
    mp_c.copy_from mp

    ptrc = mp_c.map_pointer
    ptrc.cast_to :cl_uint
    ptrc[0].should.equal 1
    ptrc[1].should.equal 2
    ptrc[5].should.equal 0xEEFF
  end

  the '#slice' do
    sz = 8 * OpenCL.type_size(:cl_uint)

    mp = Buffer.new sz, :in_out
    ptr = mp.map_pointer
    ptr.cast_to :cl_uint
    ptr.assign [0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef]

    should.raise(ArgumentError) {
      mp.slice 0, sz + 1
    }
    should.raise(ArgumentError) {
      mp.slice mp.byte_size, 1
    }
    should.raise(ArgumentError) {
      mp.slice sz / 2, sz / 2 + 1
    }
    should.not.raise(Exception) {
      mp.slice sz / 2, sz / 2
      mp.slice
      mp.slice -1
      mp.slice 0, 0
      mp.slice 1, -1
    }

    mp.slice(1, -1).should.be.nil
    mp.slice.byte_size.should.equal sz

    # To verify the contents, we can't slice from any bytes.
    smp = mp.slice 1 * OpenCL.type_size(:cl_uint), 3 * OpenCL.type_size(:cl_uint)
    smp.should.is_a Buffer
    smp.byte_size.should.equal 3 * OpenCL.type_size(:cl_uint)
    smp.should.not.be.pointer_mapped
    smp.should.be.in  # => since mp is :in_out
    smp.should.be.out

    ptrs = smp.map_pointer
    ptrs.cast_to :cl_uint
    ptrs[0].should.equal 0x34
    ptrs[1].should.equal 0x56
    ptrs[2].should.equal 0x78
  end

  the '#clone' do
    mp = Buffer.new 1024 * OpenCL.type_size(:cl_float), :in
    ptr = mp.map_pointer
    ptr.cast_to :cl_float
    ptr[0] = 1.5
    ptr[1] = 0.25
    ptr[2] = 0.125
    ptr[1023] = 3201

    mp.should.be.pointer_mapped
    mpc = mp.clone
    mp.should.not.be.pointer_mapped

    mpc.should.be.in
    mpc.should.not.be.out
    mpc.byte_size.should.equal mp.byte_size
    mpc.should.not.be.pointer_mapped

    ptrc = mpc.map_pointer
    ptrc.cast_to :cl_float
    ptrc[0].should.equal 1.5
    ptrc[1].should.equal 0.25
    ptrc[2].should.equal 0.125
    ptrc[1023].should.equal 3201

  end

end
