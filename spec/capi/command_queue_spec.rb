

require_relative '../spec_helper'

include OpenCL::Capi

describe CommandQueue do

  before do
    @devs = devices(CL_DEVICE_TYPE_ALL, platforms.first)
    @context = OpenCL::Capi::Context.new(nil, @devs)
  end

  the "constructor" do
    should.not.raise(Exception) { CommandQueue.new(@context, @devs.first, 0) }
  end

  the "info method" do
    cq = CommandQueue.new(@context, @devs.first, 0)
    should.not.raise(Exception) { cq.set_property(CL_QUEUE_PROFILING_ENABLE, true) }
    props = cq.info(CL_QUEUE_PROPERTIES)
    (props & CL_QUEUE_PROFILING_ENABLE).should.equal CL_QUEUE_PROFILING_ENABLE
    cq.set_property(CL_QUEUE_PROFILING_ENABLE, false)
    props = cq.info(CL_QUEUE_PROPERTIES)
    (props & CL_QUEUE_PROFILING_ENABLE).should.equal 0
  end

end
