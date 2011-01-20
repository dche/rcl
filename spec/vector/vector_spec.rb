# encoding: utf-8

require File.join(File.dirname(__FILE__), '../spec_helper')
require File.join(File.dirname(__FILE__), '../../lib/opencl/vector')

describe OpenCL::Vector do
  it 'should be a Buffer' do
    Vector.new(10).should.is_a OpenCL::Buffer
  end

  the 'constructor should reject invalid length' do
    should.raise(ArgumentError) { Vector.new 0.99 }
    should.raise(ArgumentError) { Vector.new -1 }
  end

  the '#length' do
    Vector.new(1).length.should.equal 1
    lh = Kernel.rand(1024)
    Vector.new(lh).length.should.equal lh
  end

  the '#type' do
    Vector.new(1).type.tag.should.equal :cl_float
  end

end

