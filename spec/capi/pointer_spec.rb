
require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL

describe HostPointer do

  before do
    @p = HostPointer.new(:cl_int, 1024);
    @scalar = [:cl_char, :cl_uchar, :cl_short, :cl_ushort, :cl_int, :cl_uint,
               :cl_long, :cl_ulong, :cl_float]
  end

  it "should reject invalid type." do
    should.raise(TypeError) { HostPointer.new 'cl_int', 1024 }
    should.raise(ArgumentError) { HostPointer.new :cl_int64, 1024 }
    should.raise(ArgumentError) { HostPointer.new :float, 2 }
  end

  it "should reject 0 size" do
    should.raise(ArgumentError) { HostPointer.new :cl_ulong, 0 }
    should.raise(ArgumentError) { HostPointer.new :cl_ulong, 1.5 }
    should.not.raise(Exception) { HostPointer.new :cl_ulong, 1 }
  end

  the 'HostPointer address should be 128 bytes aligned.' do
    (HostPointer.new(:cl_float4, 1).address & 0x7F).should.equal 0
    (@p.address & 0x7F).should.equal 0
  end

  the '#type' do
    @p.type.should.equal :cl_int

    @scalar.each do |t|
       HostPointer.new(t, 1).type.should.equal t
    end

     ['char', 'short', 'int', 'long'].map do |t|
       ['cl_' + t, 'cl_u' + t]
     end.flatten.push('cl_float').map(&:to_sym).each do |t|
       HostPointer.new(t, 123).type.should.equal t
     end
  end

  the '#type_size' do
    p = HostPointer.new :cl_uchar, 1
    p.type_size.should.equal SCALAR_TYPES[:cl_uchar]

    p = HostPointer.new :cl_float16, 4
    p.type_size.should.equal VECTOR_TYPES[:cl_float16]
  end

  the '#size' do
    @p.size.should.equal 1024

    p = HostPointer.new :cl_uint, 1
    p.size.should.equal 1

    p = HostPointer.new :cl_float2, 1
    p.size.should.equal 1
    p.byte_size.should.equal 8
    p[0] = [1, 2]
    p.size.should.equal 1
  end

  the '#byte_size' do
    @p.byte_size.should.equal 1024 * SCALAR_TYPES[:cl_int]

    p = HostPointer.new :cl_float, 1
    p.byte_size.should.equal SCALAR_TYPES[:cl_float]
    p = HostPointer.new :cl_float, 100
    p.byte_size.should.equal SCALAR_TYPES[:cl_float] * 100

  end

  the '#free' do
    p = HostPointer.new :cl_char8, 256
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

  the 'immediate values' do
    @scalar.each do |t|
      p = HostPointer.new t, 1
      p.size.should.equal 1
      p.address.should.not.be.nil
      p.free
      p.address.should.be.nil
      p.size.should.equal 0
    end
  end

  the 'getter and setter' do
    @p[1].should.equal 0
    @p[1] = 0x71115811
    @p[1].should.equal 0x71115811
  end

  the "float type" do
    p = HostPointer.new :cl_float, 300 * 300
    p[0] = 0
    p[0].should.equal 0
    p[0] = 1
    p[0].should.equal 1
    p[0] = 1.234567
    ((p[0] - 1.234567).abs < 0.001).should.be.true
  end

  the 'scalar types' do
    @scalar.each do |t|
      p = HostPointer.new t, 1
      p[0].should.equal 0
      p[0] = 1
      p[0].should.equal 1
      p = HostPointer.new t, 6
      p.assign [23, 34, 45, 56], 1
      p[1].should.equal 23
      p[2].should.equal 34
      p[3].should.equal 45
      p[4].should.equal 56
    end
  end

  the 'cl_bool type' do
    p = HostPointer.new :cl_bool, 1
    p[0].should.be.false
    p[0] = true
    p[0].should.be.true
    should.raise(TypeError) { p[0] = 1 }
  end

  the 'cl_char type' do
    p = HostPointer.new :cl_char, 200
    p[0].should.equal 0
    p[0] = 97
    p[0].should.equal 97
    p[199] = 127
    p[199].should.equal 127
    should.raise(TypeError) { p[123] = 1.234 }
  end

  the 'cl_uint4 type' do
    p = HostPointer.new :cl_uint4, 20
    p.size.should.equal 20
    p[15].should.equal [0, 0, 0, 0]
    should.not.raise(Exception) { p[15] = [1, 2, 3, 4] }
    p[15].should.equal [1, 2, 3, 4]
    p[0].should.be.instance_of Array
  end

  the 'cl_float2 type' do
    p = HostPointer.new :cl_float2, 4
    p.size.should.equal 4
    p.type.should.equal :cl_float2
    should.raise(TypeError) { p[1] = 1.234 }
    should.raise(TypeError) { p[3] = ['a', 3.0, 4] }
    should.raise(ArgumentError) { p[0] = [1] }
    should.not.raise(Exception) { p[2] = [1, 2, 3, 4] }
    p[2].should.equal [1, 2]
    p[2].should.equal [1.0, 2.0]
  end

  the '#dup and #clone' do
    @p[0] = 4345
    @p[331] = 111
    @p[1023] = 4543

    p = @p.clone
    p.size.should.equal @p.size
    p.address.should.not.equal @p.address

    p[0].should.equal 4345
    p[331].should.equal 111
    p[1023].should.equal 4543

    @p.free
    @p.size.should.equal 0
    p.size.should.not.equal 0
    should.raise(RuntimeError) { @p[0] }
    p[0].should.equal 4345
  end

  it "should be able to clone when size is 1" do
    p = HostPointer.new :cl_double, 1
    p[0] = 1.234
    q = p.dup
    q.size.should.equal 1
    q[0].should.equal 1.234
    p.free
    q.size.should.equal 1
    q[0].should.equal 1.234
  end

  the '#copy_from' do
    ps = HostPointer.new :cl_double, 200
    pd1 = HostPointer.new :cl_double, 200
    pd2 = HostPointer.new :cl_float, 200
    pd3 = HostPointer.new :cl_double, 201

    ps[0] = 1
    ps[112] = -1.234
    ps[199] = 5
    should.raise(RuntimeError) { pd2.copy_from ps }
    should.raise(RuntimeError) { pd3.copy_from ps }
    should.not.raise(Exception) { pd1.copy_from ps }
    pd1[0].should.equal 1
    pd1[112].should.equal -1.234
    pd1[199].should.equal 5
  end

  it 'should be able to copy from pointer wiht size 1' do
    ps = HostPointer.new :cl_uint, 1
    pd = HostPointer.new :cl_uint, 1
    ps[0] = 0x80008000
    pd[0].should.equal 0
    pd.copy_from ps
    pd[0].should.equal 0x80008000
    ps.free
    pd.size.should.equal 1
    pd[0].should.equal 0x80008000

    ps = HostPointer.new :cl_uchar, 2
    pd = HostPointer.new :cl_uchar, 2
    ps[0] = 1
    ps[1] = 2
    pd.copy_from ps
    pd[0].should.equal 1
    pd[1].should.equal 2
  end

  the '#slice' do
    ps = HostPointer.new :cl_uint2, 200
    ps[101] = [3, 4]
    ps[102] = [0xFFE, 0xEEF]
    pd = ps.slice(100, 4)
    ps.free
    pd.size.should.equal 4
    pd.type.should.equal :cl_uint2
    pd[0].should.equal [0, 0]
    pd[3].should.equal [0, 0]
    pd[1].should.equal [3, 4]
    pd[2].should.equal [0xFFE, 0xEEF]

    ps = HostPointer.new :cl_uchar, 3
    ps.assign [11, 12, 13]
    pd = ps.slice(1, 1)
    pd.size.should.equal 1
    pd[0].should.equal 12
  end

  it 'should be albe to slice with size 1' do
    p = HostPointer.new :cl_uchar, 1
    p[0] = 127
    p.slice(1, 1).should.be.nil
    p.slice(0, 2).should.be.nil
    q = p.slice(0, 1)
    q.should.not.be.nil
    q.should.be.instance_of HostPointer
    q.type.should.equal :cl_uchar
    q.size.should.equal 1
    q[0].should.equal 127
    p.free
    q[0].should.equal 127
  end

  the '#clear' do
    0.upto(1023) do |i|
      @p[i] = 1025 - i
    end
    @p.clear
    0.upto(1023) do |i|
      @p[i].should.equal 0
    end

    p = HostPointer.new :cl_uchar, 1
    p[0] = 97
    p.clear
    p[0].should.equal 0

    p = HostPointer.new(:cl_uint2, 3)
    p[2] = [1, 2]
    p.clear
    p[2].should.equal [0, 0]
  end

  the '#assign_pointer' do
    src = HostPointer.new :cl_char, 1
    dst = HostPointer.new :cl_char, 100
    dst[22].should.equal 0
    src[0] = 0xef;
    dst.assign_pointer src.address, 1, 22
    dst[22].should.equal -17

    dst1 = HostPointer.new :cl_uchar, 3
    dst1.assign_pointer src.address, 1, 0
    dst1[0].should.equal 0xef
    dst1[1].should.equal 0
  end

  the '#assign_byte_string' do
    p = HostPointer.new :cl_uchar4, 1
    bytes = [1, 2, 3, 4].pack('C4')
    p.assign_byte_string bytes, 0
    p[0].should.equal [1, 2, 3, 4]

    should.raise(ArgumentError) { p.assign_byte_string "abc", 0 }
    should.not.raise(ArgumentError) { p.assign_byte_string "abcd", 0 }
    p[0].should.equal [97, 98, 99, 100]
    should.raise(TypeError) { p.assign_byte_string [1, 2, 3, 4], 0}
    should.raise(ArgumentError) { p.assign_byte_string bytes, 1}
    p.size.should.equal 1
  end

  the '#assign' do
    p = HostPointer.new :cl_uint, 8
    p.assign [1, 2]
    p[0].should.equal 1
    p[1].should.equal 2
    p.assign [3, 4], 2
    p[0].should.equal 1
    p[1].should.equal 2
    p.assign [3], 0
    p[0].should.equal 3
    p[1].should.equal 2
    p.assign [0, 0], 1
    p[0].should.equal 3
    p[1].should.equal 0

    p = HostPointer.new :cl_uchar, 7
    p.assign [1, 2]
    p[0].should.equal 1
    p[1].should.equal 2
    p.assign [0xef, 0xef, 0xef], 6
    p[6].should.equal 0xef
  end

  the '::wrap_pointer' do
    p = HostPointer.new :cl_float, 20
    wp = HostPointer.wrap_pointer p.address, :cl_float, 10
    wp.should.is_a HostPointer
    wp.byte_size.should.equal SCALAR_TYPES[:cl_float] * 10
    p[0] = 1.1
    wp[0].should.equal p[0]
    wp.free
    wp.size.should.equal 0

    p = HostPointer.new :cl_short, 1
    wp = HostPointer.wrap_pointer p.address, :cl_short, 1
    wp[0].should.equal p[0]
    wp.byte_size.should.equal 2
    p[0] = -32757
    wp[0].should.equal p[0]
    wp.free
    wp.should.be.null
    p[0].should.equal -32757
  end
end
