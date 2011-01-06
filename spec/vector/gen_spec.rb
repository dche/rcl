# encoding: utf-8

require File.join(File.dirname(__FILE__), '../spec_helper')
require File.join(File.dirname(__FILE__), '../../lib/opencl/vector')

describe "Generators of OpenCL::Vector" do
  the 'default value should be zero' do
    v = Vector.new 2, :cl_uint
    v[0].should.equal 0
    v[1].should.equal 0

    len = 2 ** 20
    v = Vector.new len
    5.times do
      i = Kernel.rand len
      v[i].should.equal 0
    end

    v = Vector.new len
    5.times do
      i = Kernel.rand len
      v[i].should.equal 0
    end

  end

  it 'should fill the vector with a single value' do
    v = Vector.ones 32, :cl_uint
    5.times do
      i = Kernel.rand v.length
      v[i].should.equal 1
    end

    v = Vector.new(2 ** 20).fill(-1234.125)
    5.times do
      i = Kernel.rand v.length
      v[i].should.equal -1234.125
    end
  end

  describe '#range' do
    the 'Normal Integer' do
      v = Vector.range(1..10, 1, :cl_int)
      v.length.should.equal 10
      v[0].should.equal 1
      v[1].should.equal 2
      v[9].should.equal 10
    end

    the 'Decreased Integer' do
      v = Vector.range(10..1)
      v.length.should.equal 10
      v[0].should.equal 10
      v[1].should.equal 9
      v[9].should.equal 1
    end

    the 'Fraction step' do
      v = Vector.range(1..10, 0.5)
      v.length.should.equal 19
      v[0].should.equal 1
      v[1].should.equal 1.5
      v[2].should.equal 2
      v[17].should.equal 9.5
      v[18].should.equal 10

      v.fill_range(2, 0.25)
      v.length.should.equal 19
      v[0].should.equal 2
      v[1].should.equal 2.25
      v[2].should.equal 2.5
      v[17].should.equal 6.25
      v[18].should.equal 6.5
    end

    the 'Decandent fraction step' do
      v = Vector.new(20).fill_range(5, -0.25)
      v[0].should.equal 5
      v[1].should.equal 4.75
      v[2].should.equal 4.5
      v[17].should.equal 0.75
      v[18].should.equal 0.5
      v[19].should.equal 0.25

      v = Vector.range 5..0.25, -0.25
      v.length.should.equal 20
      v[0].should.equal 5
      v[1].should.equal 4.75
      v[2].should.equal 4.5
      v[17].should.equal 0.75
      v[18].should.equal 0.5
      v[19].should.equal 0.25
    end
  end

end
