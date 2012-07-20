
require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL::Capi

describe MappedPointer do
  before do
    @cxt = OpenCL::Capi::Context.new([CL_CONTEXT_PLATFORM, platforms.first], CL_DEVICE_TYPE_ALL)
  end

  it "should not be instantiated." do
    should.raise(Exception) { MappedPointer.new }
    should.raise(Exception) { MappedPointer.allocate }
  end

  the "cast_to() method" do
    mem = @cxt.create_buffer(CL_MEM_READ_WRITE, 16, nil)

    cq = @cxt.create_command_queue
    mp1 = cq.enqueue_map_buffer(mem, true, CL_MAP_READ, 0, 3, nil).first
    mp1.should.is_a MappedPointer
    should.raise(Exception) { mp1.cast_to :cl_int }
    should.not.raise(Exception) { mp1.cast_to :cl_uchar }
    should.not.raise(Exception) { mp1.cast_to :cl_char }
    should.raise(Exception) { mp1.cast_to :cl_short }
    should.raise(Exception) { mp1.cast_to :cl_uchar2}

    mp2, evt = cq.enqueue_map_buffer(mem, false, CL_MAP_READ, 0, 16, nil)
    cq.enqueue_wait_for_events [evt]
    mp2.should.is_a MappedPointer
    mp2.size.should.equal 16
    mp2.type.should.equal :cl_uchar
    should.not.raise(Exception) { mp2.cast_to :cl_int }
    mp2.type.should.equal :cl_int
    mp2.size.should.equal 4
    should.not.raise(Exception) { mp2.cast_to :cl_ushort }
    mp2.size.should.equal 8
    mp2.type.should.equal :cl_ushort
    should.not.raise(Exception) { mp2.cast_to :cl_uchar16 }
    mp2.size.should.equal 1
    should.not.raise(Exception) { mp2.cast_to :cl_uint2 }
    mp2.size.should.equal 2
    should.not.raise(Exception) { mp2.cast_to :cl_int4 }
    mp2.size.should.equal 1
    should.not.raise(Exception) { mp2.cast_to :cl_float }
    mp2.size.should.equal 4
    should.not.raise(Exception) { mp2.cast_to :cl_double }
    mp2.size.should.equal 2
    should.not.raise(Exception) { mp2.cast_to :cl_ulong }
    mp2.size.should.equal 2

    cq.enqueue_unmap_mem_object(mem, mp1, nil)
    mp1.should.be.null
    mp2.should.not.be.null
    cq.enqueue_unmap_mem_object(mem, mp2, nil)
    cq.finish
    mp2.should.be.null
    mp2.size.should.equal 0
    should.not.raise(Exception) { mp2.cast_to :cl_uchar }
    mp2.size.should.equal 0
  end

  it "should be invalidate after unmap." do
    mem = @cxt.create_buffer(CL_MEM_READ_WRITE, 16, nil)

    cq = @cxt.create_command_queue
    mp = cq.enqueue_map_buffer(mem, true, CL_MAP_READ, 0, 2, nil).first
    mp.should.is_a MappedPointer
    mp.should.not.be.null
    mp.size.should.equal 2
    mp.type.should.equal :cl_uchar

    cq.enqueue_unmap_mem_object(mem, mp, nil)
    cq.finish

    mp.should.be.null
    mp.size.should.equal 0
  end

  the "get/set and assign for allocated pointer." do
    mem = @cxt.create_buffer(CL_MEM_READ_WRITE, 16, nil)

    cq = @cxt.create_command_queue
    mp = cq.enqueue_map_buffer(mem, true, CL_MAP_READ, 0, 16, nil).first

    mp.assign [1, 2, 3], 3
    mp[3].should.equal 1
    mp[4].should.equal 2
    mp[5].should.equal 3

    mp.assign [0xfe], 15
    mp[15].should.equal 0xfe

    mp.cast_to :cl_ulong
    mp.clear
    mp.assign [0xffeeff]
    mp[0].should.equal 0xffeeff

    cq.enqueue_unmap_mem_object(mem, mp, nil)
    cq.finish
  end

  the '#clear' do
    mem = @cxt.create_buffer(CL_MEM_READ_WRITE, 16, nil)

    cq = @cxt.create_command_queue
    mp = cq.enqueue_map_buffer(mem, true, CL_MAP_READ, 0, 16, nil).first

    mp.assign [1, 2, 3], 3
    mp.clear

    mp[3].should.equal 0
    mp[4].should.equal 0
    mp[5].should.equal 0
  end

  the '#dirty? and #clear_dirty' do
    mem = @cxt.create_buffer(CL_MEM_READ_WRITE, 16, nil)

    cq = @cxt.create_command_queue
    mp = cq.enqueue_map_buffer(mem, true, CL_MAP_READ, 0, 16, nil).first
    mp.should.not.be.dirty
    mp.assign [1, 3, 4]
    mp.should.be.dirty
    mp.clear_dirty
    mp.should.not.be.dirty
    mp[0] = 0
    mp.should.be.dirty
    mp.clear_dirty
    mp.assign "abc"
    mp.should.be.dirty
  end

  the '#mark_dirty' do
    mem = @cxt.create_buffer(CL_MEM_READ_WRITE, 16, nil)
    cq = @cxt.create_command_queue
    mp = cq.enqueue_map_buffer(mem, true, CL_MAP_READ, 0, 16, nil).first
    mp.should.not.be.dirty
    mp.mark_dirty
    mp.should.be.dirty
  end

  the '#read_as_type and #write_as_type' do
    mem = @cxt.create_buffer(CL_MEM_READ_WRITE, 16, nil)
    cq = @cxt.create_command_queue
    mp = cq.enqueue_map_buffer(mem, true, CL_MAP_READ, 0, 16, nil).first
    mp.cast_to :cl_uint4
    mp[0] = [2, 3, 5, 7]
    mp.read_as_type(4, :cl_uint).should.equal 3
    mp.write_as_type(8, :cl_uint, 6)
    mp[0].should.equal [2, 3, 6, 7]
    should.raise(ArgumentError) {
      mp.read_as_type(13, :cl_uint)
    }
    should.raise(ArgumentError) {
      mp.write_as_type(12, :cl_float2)
    }
    should.not.raise(ArgumentError) {
      mp.read_as_type(13, :cl_ushort)
      mp.write_as_type(15, :cl_char, 5)
    }
  end
end
