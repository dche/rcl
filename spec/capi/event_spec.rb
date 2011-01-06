# encoding: utf-8

require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL::Capi

describe Event do
  before do
    @cxt = OpenCL::Capi::Context.new(nil, CL_DEVICE_TYPE_DEFAULT)
  end

  the "info() method." do
    hp = HostPointer.new :cl_double4, 1
    mem = @cxt.create_buffer(CL_MEM_READ_ONLY, hp.byte_size)

    cq = @cxt.create_command_queue
    event = cq.enqueue_write_buffer(mem, false, 0, hp.byte_size, hp, nil)

    event.should.is_a Event
    event.info(CL_EVENT_COMMAND_TYPE).should.equal CL_COMMAND_WRITE_BUFFER

    Capi.wait_for_events([event])
    event.info(CL_EVENT_COMMAND_EXECUTION_STATUS).should.equal CL_COMPLETE
  end

  the "#profiling_info" do
    hp = HostPointer.new :cl_double4, 1
    mem = @cxt.create_buffer(CL_MEM_READ_ONLY, hp.byte_size)

    cq = @cxt.create_profiling_command_queue
    event = cq.enqueue_write_buffer(mem, false, 0, hp.byte_size, hp, nil)
    cq.finish

    st = 0
    should.not.raise(Exception) {
      st = event.profiling_info(CL_PROFILING_COMMAND_START)
    }
    (st > 0).should.be.true

    event.info(CL_EVENT_COMMAND_EXECUTION_STATUS).should.equal CL_COMPLETE
    et = event.profiling_info(CL_PROFILING_COMMAND_END)
    (et > st).should.be.true
  end

  # the "#set_callback" do
  #   hp = HostPointer.new :cl_double4, 1
  #   mem = @cxt.create_buffer(CL_MEM_READ_ONLY, hp.byte_size)
  #
  #   cq = @cxt.create_profiling_command_queue
  #   event = cq.enqueue_write_buffer(mem, false, 0, hp.byte_size, hp, nil)
  #
  #   status = CL_RUNNING
  #   event.set_callback(CL_QUEUED) do |event, cmd_exec_status|
  #     status = cmd_exec_status
  #   end
  #   # ensure the callback is called.
  #   sleep 0.1
  #   status.should.equal CL_QUEUED
  #
  #   blk = Proc.new do |event, cmd_exec_status|
  #     status = cmd_exec_status
  #   end
  #
  #   # default status to monitor is CL_COMPLETE
  #   event.set_callback &blk
  #   sleep 0.1
  #   status.should.equal CL_COMPLETE
  #
  # end
end
