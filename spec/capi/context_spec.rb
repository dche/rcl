

require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL::Capi

# NOTE: It seems that NVIDIA's OpenCL implementatin does not allow contexts
#       share devices. A CL_DEVICE_NOT_AVAILABLE error is returned.
platform = platforms.first
cxt = OpenCL::Capi::Context.new([CL_CONTEXT_PLATFORM, platform], CL_DEVICE_TYPE_DEFAULT)

describe Context do

  the "constructor" do
    cxt.should.is_a OpenCL::Capi::Context
    OpenCL::Capi::Context.new(nil, devices(CL_DEVICE_TYPE_ALL, platform)).should.is_a OpenCL::Capi::Context

    should.raise(ArgumentError) { OpenCL::Capi::Context.new(nil, []) }
    should.raise(ArgumentError) { OpenCL::Capi::Context.new([CL_CONTEXT_PLATFORM, platforms.first], "string") }
  end

  the "info() method" do
    cxt.info(CL_CONTEXT_REFERENCE_COUNT).should.is_a Integer
    devs = cxt.info(CL_CONTEXT_DEVICES)
    devs.should.is_a Array
    devs.should.not.be.empty
    devs.first.should.is_a Device
    puts devs.first.info(CL_DEVICE_NAME)
  end

  the "retainContext should performed." do
    rc = cxt.info(CL_CONTEXT_REFERENCE_COUNT)

    cxt2 = cxt.dup
    rc1 = cxt.info(CL_CONTEXT_REFERENCE_COUNT)
    rc2 = cxt2.info(CL_CONTEXT_REFERENCE_COUNT)

    (rc1 > rc).should.be.true
    (rc1 == rc2).should.be.true
  end
end
