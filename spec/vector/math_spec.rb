# encoding: utf-8

require File.join(File.dirname(__FILE__), '../spec_helper')
require File.join(File.dirname(__FILE__), '../../lib/opencl/vector')

describe "Reduction functions of OpenCL::Vector" do

  min = 1
  max = rand(16384)

  before do
    @vec = Vector.range(min..max)
  end

  the '#min, #max' do
    @vec.min.should.equal min
    @vec.max.should.equal max
  end

  the '#mad should check types and vector lengths' do
    should.raise(ArgumentError) {
      @vec.mad(Vector.new(1024), Vector.new(@vec.length))
    }
    should.raise(ArgumentError) {
      @vec.mad(Vector.new(@vec.length), @vec.length)
    }
    should.not.raise(ArgumentError) {
      @vec.mad(Vector.new(@vec.length), Vector.ones(@vec.length))
    }
    should.raise(TypeError) {
      @vec.mad(10, Vector.ones(@vec.length))
    }
  end

  the '#mad numbers' do
    @vec.mad(10, 1)
    @vec[0].should.equal 11
    @vec[1].should.equal 21
    @vec[2].should.equal 31
    @vec[@vec.length - 1].should.equal (max * 10 + 1)
  end

  the '#mad vectors' do
    mvec = Vector.new(@vec.length).fill(10)
    avec = Vector.ones(@vec.length)
    @vec.mad(mvec, avec)
    @vec[0].should.equal 11
    @vec[1].should.equal 21
    @vec[2].should.equal 31
    @vec[@vec.length - 1].should.equal (max * 10 + 1)
  end

  the '#power' do
    @vec.power 3
    @vec[0].should.equal 1
    @vec[1].should.equal 8
    @vec[2].should.equal 27
  end

  the '#square' do
    @vec.square
    @vec[0].should.equal 1
    @vec[1].should.equal 4
    @vec[2].should.equal 9
  end

  the '#abs' do
    (@vec * -1).abs
    @vec[0].should.equal 1
    @vec[1].should.equal 2
    @vec[2].should.equal 3
    @vec[@vec.length - 1].should.equal max
  end

end

