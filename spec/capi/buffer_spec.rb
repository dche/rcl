
require File.join(File.dirname(__FILE__), '../spec_helper')
include OpenCL::Capi

describe Memory do

  before do
    @cxt = OpenCL::Capi::Context.new([CL_CONTEXT_PLATFORM, platforms.first], CL_DEVICE_TYPE_DEFAULT)
  end

  it "should create buffer object." do
    mem = @cxt.create_buffer(CL_MEM_WRITE_ONLY, 1024, nil)
    mem.should.is_a Memory
    mem.info(CL_MEM_SIZE).should.equal 1024
    # mem.info(CL_MEM_HOST_PTR).should.be.nil
  end

  it "should create buffer with host pointer" do
    hp = HostPointer.new(:cl_uint, 256)
    mem = Memory.new(:buffer, @cxt, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, 0, hp)
    mem.should.is_a Memory
    mem.info(CL_MEM_SIZE).should.equal 1024
    mem.info(CL_MEM_HOST_PTR).should.equal hp.address
  end

  the "write and read buffer." do
    hp = HostPointer.new(:cl_uint4, 8);
    hp[0] = [1, 2, 3, 4]
    hp[7] = [4, 3, 2, 1]

    mem = @cxt.create_buffer(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 0, hp)
    mem.should.is_a Memory

    cq = @cxt.create_command_queue
    cq.should.is_a CommandQueue

    cq.enqueue_write_buffer(mem, true, 0, hp.byte_size, hp, nil)

    hp.clear
    hp[0].should.equal [0, 0, 0, 0]
    hp[7].should.equal [0, 0, 0, 0]

    event = cq.enqueue_read_buffer(mem, false, 0, hp.byte_size, hp, nil)
    Capi.wait_for_events [event]
    hp[0].should.equal [1, 2, 3, 4]
    hp[7].should.equal [4, 3, 2, 1]
  end

  the "copy buffer." do
    hp1 = HostPointer.new :cl_double, 2
    hp2 = HostPointer.new :cl_double, 2

    hp1[0] = 1.0
    hp1[1] = 1.25
    hp2[0].should.equal 0

    mem1 = @cxt.create_buffer(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 0, hp1)
    mem2 = @cxt.create_buffer(CL_MEM_READ_WRITE, hp2.byte_size, nil)

    cq = @cxt.create_command_queue
    e = cq.enqueue_copy_buffer(mem1, mem2, 0, 0, hp2.byte_size, nil)
    cq.enqueue_read_buffer(mem2, true, 0, hp2.byte_size, hp2, [e])

    hp2[0].should.equal 1.0
    hp2[1].should.equal 1.25
  end

  the "non blocking write." do
    hp = HostPointer.new :cl_double4, 1
    mem = Memory.new(:buffer, @cxt, CL_MEM_READ_ONLY, hp.byte_size, nil)

    cq = @cxt.create_command_queue
    event = cq.enqueue_write_buffer(mem, false, 0, hp.byte_size, hp, nil)

    event.should.is_a Event
    should.not.raise(Exception) { cq.enqueue_wait_for_events [event] }
  end

  the 'map and unmap buffer' do
    hp = HostPointer.new :cl_uint, 1
    hp[0] = 0xEFEF0000
    mem = @cxt.create_buffer(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 0, hp)

    cq = @cxt.create_command_queue
    mp = cq.enqueue_map_buffer(mem, true, CL_MAP_READ, 0, hp.byte_size, nil).first
    mp.should.is_a MappedPointer
    mp.cast_to(:cl_uint)[0].should.equal 0xEFEF0000
    mc = mem.info(CL_MEM_MAP_COUNT)
    should.not.raise(Exception) { cq.enqueue_unmap_mem_object(mem, mp, nil) }
    cq.finish
    (mem.info(CL_MEM_MAP_COUNT) < mc).should.be.true
  end

end
