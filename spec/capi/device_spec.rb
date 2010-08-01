
require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL::Capi

describe Device do
  it "should not be instantiated." do
    should.raise(Exception) { Device.new }
  end
  
  the "devices() method." do
    devices(CL_DEVICE_TYPE_GPU, nil).should.is_a Array
    devices(CL_DEVICE_TYPE_CPU, nil).should.is_a Array
    devices(CL_DEVICE_TYPE_ALL, platforms.first).should.is_a Array
    
    alldevs = devices(CL_DEVICE_TYPE_ALL, nil)
    alldevs.should.is_a Array
    alldevs.first.should.is_a Device
  end

  the "info() method should reject invalid param" do
    d = devices(CL_DEVICE_TYPE_ALL, nil).first
    should.raise(TypeError) { d.info("error") }
  end
  
  the "info() method should return value according to the param type." do
    d = devices(CL_DEVICE_TYPE_ALL, nil).first
    d.info(CL_DEVICE_NAME).should.is_a String
    d.info(CL_DEVICE_AVAILABLE).should.be.true
    d.info(CL_DEVICE_ADDRESS_BITS).should.is_a Fixnum
    d.info(CL_DEVICE_IMAGE2D_MAX_WIDTH).should.is_a Fixnum
    d.info(CL_DEVICE_MAX_WORK_ITEM_SIZES).should.is_a Array
    d.info(CL_DEVICE_PLATFORM).should.is_a Platform
    puts d.info(CL_DEVICE_PLATFORM).info(CL_PLATFORM_NAME)
  end

end
