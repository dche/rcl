# encoding: utf-8

require File.join(File.dirname(__FILE__), 'spec_helper')

describe OpenCL do

  the '::type_size' do
    OpenCL.type_size(:cl_float).should.equal 4
    OpenCL.type_size(:cl_uchar16).should.equal 16
    OpenCL.type_size(:invalid_type).should.equal 0
    OpenCL.type_size('invalid arg').should.equal 0
  end

  the '::valid_type?' do
    OpenCL.valid_type?(:cl_uint).should.be.true
    OpenCL.valid_type?(:cl_float4).should.be.true
    OpenCL.valid_type?(1234).should.be.false
    OpenCL.valid_type?([]).should.be.false
  end

  the '::valid_vector?' do
    OpenCL.valid_vector?(:cl_ulong).should.be.false
    OpenCL.valid_vector?(:cl_float4).should.be.true
    OpenCL.valid_vector?(:cl_ulong8).should.be.true
    OpenCL.valid_vector?(:invalid_type).should.be.false
  end
end
