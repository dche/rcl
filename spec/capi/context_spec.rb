

require_relative '../spec_helper'

include OpenCL::Capi

describe Context do
  the "constructor" do
    OpenCL::Capi::Context.new(nil, CL_DEVICE_TYPE_CPU).should.is_a OpenCL::Capi::Context
    OpenCL::Capi::Context.new(nil, devices(CL_DEVICE_TYPE_ALL, nil)).should.is_a OpenCL::Capi::Context
    OpenCL::Capi::Context.new(platforms.first, CL_DEVICE_TYPE_CPU).should.is_a OpenCL::Capi::Context

    should.raise(ArgumentError) { OpenCL::Capi::Context.new(nil, []) }
    should.raise(ArgumentError) { OpenCL::Capi::Context.new(nil, "string") }
  end

  the "info() method" do
    cxt = OpenCL::Capi::Context.new(nil, CL_DEVICE_TYPE_DEFAULT);
    cxt.info(CL_CONTEXT_REFERENCE_COUNT).should.is_a Integer
    devs = cxt.info(CL_CONTEXT_DEVICES)
    devs.should.is_a Array
    devs.should.not.be.empty
    devs.first.should.is_a Device
    puts devs.first.info(CL_DEVICE_NAME)
  end

  the "retainContext should performed." do
    cxt = OpenCL::Capi::Context.new(nil, CL_DEVICE_TYPE_DEFAULT)
    rc = cxt.info(CL_CONTEXT_REFERENCE_COUNT)

    cxt2 = cxt.dup
    rc1 = cxt.info(CL_CONTEXT_REFERENCE_COUNT)
    rc2 = cxt2.info(CL_CONTEXT_REFERENCE_COUNT)

    (rc1 > rc).should.be.true
    (rc1 == rc2).should.be.true
  end
end
