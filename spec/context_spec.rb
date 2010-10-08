
require File.join(File.dirname(__FILE__), 'spec_helper')

include OpenCL

describe OpenCL::Context do

  the '::new should be private' do
    should.raise(NoMethodError) { OpenCL::Context.new :gpu }
  end

  the '::context_of' do
    cxt = OpenCL::Context.context_of :cpu
    cxt.should.is_a OpenCL::Context

    devs = cxt.devices
    devs.should.not.be.empty

    devs.first.should.is_a Capi::Device
    devs.first.should.not.be.gpu
  end

  the '::default_context' do
    cxt = OpenCL::Context.default_context
    cxt.should.is_a OpenCL::Context

    cxt.devices.should.not.be.empty
  end

  the '#default_device' do
    cxt = OpenCL::Context.default_context
    cxt.default_device.should.is_a Capi::Device
  end

  the '#max_mem_alloc_size' do
    cxt = OpenCL::Context.default_context
    (cxt.max_mem_alloc_size <= cxt.default_device.max_mem_alloc_size).should.be.true
    (cxt.max_mem_alloc_size > 0).should.be.true
  end

  the '#command_queue_of' do
    cxt = OpenCL::Context.default_context
    cq = cxt.command_queue_of cxt.default_device
    cq.should.is_a Capi::CommandQueue
  end

  the '#profiling_command_queue_of' do
    cxt = OpenCL::Context.default_context
    cq = cxt.profiling_command_queue_of cxt.default_device
    cq.should.is_a Capi::CommandQueue
    cq.info(Capi::CL_QUEUE_PROPERTIES).should.equal Capi::CL_QUEUE_PROFILING_ENABLE
  end
end
