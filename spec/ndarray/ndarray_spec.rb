# encoding: utf-8

require File.join(File.dirname(__FILE__), '../spec_helper')

describe NDArray do
  given 'invalid shapes' do
    it 'should raise ArgumentError' do
      should.raise(ArgumentError) {
        NDArray.new 1, :cl_float
      }
      should.raise(ArgumentError) {
        NDArray.new [1, 0], :cl_float
      }
      should.raise(ArgumentError) {
        NDArray.new [1, -1, 1], :cl_float
      }
      should.raise(ArgumentError) {
        NDArray.new [1, -1, 1], :cl_float
      }
    end
  end

  given 'valid shapes' do
    it 'should not raise any Exception' do
      should.not.raise(Exception) {
        NDArray.new [1], :cl_char
        NDArray.new [2, 3]
        NDArray.new [1, 2, 3, 4, 5], :cl_float4
        NDArray.new ['1', '2']
        NDArray.new [1, [2]], :cl_int
      }
    end
  end

  the '#shape' do
    NDArray.new([1]).shape.should.equal [1]
    NDArray.new(['1', 2]).shape.should.equal [1, 2]
  end

  the '#dim' do
    NDArray.new([1]).dim.should.equal 1
    NDArray.new([2, 3]).dim.should.equal 2
    NDArray.new([4,5,6,7,8]).dim.should.equal 5
  end
end
