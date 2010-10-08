
require File.join(File.dirname(__FILE__), '../spec_helper')
require File.join(File.dirname(__FILE__), '../../lib/opencl/operand')

include OpenCL

class MyVector < Operand

  use_cl_modules :random

  add_kernels <<-EOK
    __kernel void
    my_vector_add(__global float *vec, float number)
    {
        int gid = get_global_id(0);

        vec[gid] += number;
    }
  EOK

  def cl_add(number)
    call_cl_method :my_vector_add, [self.length], :mem, self, :cl_float, number
  end

  def [](i)
    super(i)
  end

  def []=(i, x)
    super(i, x)
  end

end

describe Operand do
  before do
    @out = MyVector.new 10, :cl_float
  end

  it 'should is a Buffer' do
    @out.should.is_a OpenCL::Buffer
  end

  its '#size and #length' do
    @out.length.should.equal 10
    @out.size.should.equal @out.length

    sz = Kernel.rand(1024)
    MyVector.new(sz).length.should.equal sz
  end

  its '#type' do
    @out.type.should.equal :cl_float
    MyVector.new(10, :cl_uint).type.should.equal :cl_uint
  end

  it 'should respond to defined cl methods.' do
    @out.should.respond_to :cl_add
  end

  it 'should be computed.' do
    ln = @out.length
    ln.times do |i|; @out[i] = i; end
    @out.cl_add(1)
    1.upto(ln) do |i|; @out[i - 1].should.equal(i); end
  end

  the 'setter should complain if valud type is mismatch.' do
    should.raise(TypeError) { @out[0] = '1.234' }
    should.not.raise(Exception) { @out[0] = '1.234'.to_f }
  end

  the 'setter and getter should complain if subscriber is too large.' do
    should.raise(RuntimeError) { @out[@out.length] }
    should.raise(RuntimeError) { @out[@out.length] = 1 }
  end

  the '::cl_modules_in_use' do
    MyVector.cl_modules_in_use.should.include(:random)
  end

end
