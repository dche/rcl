

require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL::Capi

describe Context do
  the "constructor" do
    Context.new(nil, CL_DEVICE_TYPE_CPU).should.is_a Context    
    Context.new(nil, devices(CL_DEVICE_TYPE_ALL, nil)).should.is_a Context
    Context.new(platforms.first, CL_DEVICE_TYPE_CPU).should.is_a Context
    
    should.raise(ArgumentError) { Context.new(nil, []) }
    should.raise(ArgumentError) { Context.new(nil, "string") }
  end
  
  the "info() method" do
    
  end
end
