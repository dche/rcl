
require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL::Capi

describe Device do
  it "should not be instantiated." do
    should.raise(RuntimeError) { Device.new }
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
    should.raise(ArgumentError) { d.info("error") }
  end
  
  the "info() method should return value according to the param type." do
    
  end

end
