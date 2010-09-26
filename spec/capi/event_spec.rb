
require_relative '../spec_helper'

include OpenCL
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
    
    should.not.raise(Exception) { cq.enqueue_wait_for_events [event] }
    # FIXME: bug in wait_for_events()? See also buffer_spec.rb
    cq.finish
    event.info(CL_EVENT_COMMAND_EXECUTION_STATUS).should.equal CL_COMPLETE
  end
  
end
