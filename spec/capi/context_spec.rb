

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
    cxt = Context.new(nil, CL_DEVICE_TYPE_CPU);
    cxt.info(CL_CONTEXT_REFERENCE_COUNT).should.is_a Integer
  end
  
  the "retainContext should performed." do
    cxt = Context.new(nil, CL_DEVICE_TYPE_CPU)
    rc = cxt.info(CL_CONTEXT_REFERENCE_COUNT)
    
    cxt2 = cxt.dup
    rc1 = cxt.info(CL_CONTEXT_REFERENCE_COUNT)
    rc2 = cxt2.info(CL_CONTEXT_REFERENCE_COUNT)
    
    (rc1 > rc).should.be.true
    (rc1 == rc2).should.be.true
  end
end
