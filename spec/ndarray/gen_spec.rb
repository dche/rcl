# encoding: utf-8

require File.join(File.dirname(__FILE__), '../spec_helper')
require File.join(File.dirname(__FILE__), '../../lib/opencl/ndarray')

describe NDArray do
  the '::like' do
    na1 = NDArray.new [1,2,3,4,5], :cl_uchar4
    na = NDArray.like(na1)

    na.dim.should.equal 5
    na.shape.should.equal [1,2,3,4,5]
    na.type.tag.should.equal :cl_uchar4
  end
end
