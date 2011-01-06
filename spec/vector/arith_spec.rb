# encoding: utf-8

require File.join(File.dirname(__FILE__), '../spec_helper')
require File.join(File.dirname(__FILE__), '../../lib/opencl/vector')

describe "Arithmetics of OpenCL::Vector" do
  min = 1
  max = min + 2 + rand(4096)

  before do
    @vec = Vector.range(min..max)
  end

  the '#summary' do
    ra = (min..max).to_a
    @vec.summary.should.equal ra.reduce(:+)
  end

  it 'should complain if two vectors have different length.' do
    vec = Vector.new (max - 1)
    should.raise(ArgumentError) { @vec + vec }
    should.raise(ArgumentError) { vec + @vec }
    should.raise(ArgumentError) { @vec - vec }
    should.raise(ArgumentError) { vec - @vec }
  end

  the 'Add a vector' do
    vec = Vector.range(min..max)
    @vec + vec
    @vec[0].should.equal 2
    @vec[1].should.equal 4
    @vec[2].should.equal 6
    @vec[vec.length - 1].should.equal max * 2
  end

  the 'Add a scalar' do
    @vec + 10
    @vec[0].should.equal 11
    @vec[1].should.equal 12
    @vec[@vec.length - 1].should.equal max + 10
  end

  the 'Subtract a vector' do
    vec = Vector.range(min..max)
    @vec - vec
    @vec[0].should.equal 0
    @vec[1].should.equal 0
    @vec[2].should.equal 0
    @vec[vec.length - 1].should.equal 0
  end

  the 'Subtract a scalar' do
    @vec - 1
    @vec[0].should.equal 0
    @vec[1].should.equal 1
    @vec[@vec.length - 1].should.equal (max - 1)
  end

  the '#multiply' do
    @vec * 10
    @vec[0].should.equal 10
    @vec[1].should.equal 20
    @vec[2].should.equal 30
    @vec[@vec.length - 1].should.equal max * 10
  end

  the 'Multiply a Vector' do
    vec = Vector.new(@vec.length).fill(10)
    @vec * vec
    @vec[0].should.equal 10
    @vec[1].should.equal 20
    @vec[2].should.equal 30
    @vec[@vec.length - 1].should.equal max * 10
  end

  the '#divide should not.complain if the divider is 0' do
    should.not.raise(ArgumentError) { @vec / 0 }
    @vec[0].should.be.infinite
  end

  the '#divide' do
    vec = Vector.range(0..4096, 2, :cl_ushort) / 2
    vec[0].should.equal 0
    vec[1].should.equal 1
    vec[vec.length - 1].should.equal 2048
  end

end
