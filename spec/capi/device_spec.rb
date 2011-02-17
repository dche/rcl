
require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL::Capi

describe Device do
  before do
    @platform = platforms.first
  end

  it "should not be instantiated." do
    should.raise(Exception) { Device.new }
  end

  the "devices() method." do
    devices(CL_DEVICE_TYPE_GPU, @platform).should.is_a Array
    devices(CL_DEVICE_TYPE_CPU, @platform).should.is_a Array
    devices(CL_DEVICE_TYPE_ALL, @platform).should.is_a Array

    alldevs = devices(CL_DEVICE_TYPE_ALL, @platform)
    alldevs.should.is_a Array
    alldevs.first.should.is_a Device
  end

  the '#eql?' do
    # CHECK: weak condition. depends on the order of devices.
    d1 = devices(CL_DEVICE_TYPE_DEFAULT, @platform).first
    d2 = devices(CL_DEVICE_TYPE_DEFAULT, @platform).first

    d1.object_id.should.not.equal d2.object_id
    d1.should.eql d2
  end

  the "info() method should reject invalid param" do
    d = devices(CL_DEVICE_TYPE_ALL, @platform).first
    should.raise(TypeError) { d.info("error") }
  end

  the "info() method should return value according to the param type." do
    d = devices(CL_DEVICE_TYPE_DEFAULT, @platform).first
    d.info(CL_DEVICE_NAME).should.is_a String
    d.info(CL_DEVICE_AVAILABLE).should.be.true
    d.info(CL_DEVICE_ADDRESS_BITS).should.is_a Fixnum
    d.info(CL_DEVICE_IMAGE2D_MAX_WIDTH).should.is_a Fixnum
    d.info(CL_DEVICE_MAX_WORK_ITEM_SIZES).should.is_a Array
    d.info(CL_DEVICE_PLATFORM).should.is_a Platform
  end

end
