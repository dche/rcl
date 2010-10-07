require_relative '../spec_helper'

include OpenCL
include OpenCL::Capi

src = <<-EOF
  __kernel void
  dot_product (__global const float4 *a,
               __global const float4 *b,
               __global float *c)
  {
    int gid = get_global_id(0);

    c[gid] = dot(a[gid], b[gid]);
  }
EOF

wrong_src = <<-EOF
  __kernel void
  dot_product (__global const float4 *a,
               __global const float4 *b,
               __global float *c)
  {
    int gid = get_global_id(0)

    c[gid] = dot(a[gid], b[gid]);
  }
EOF

describe Program do
  before do
    @cxt = OpenCL::Capi::Context.new(nil, CL_DEVICE_TYPE_DEFAULT)
  end

  the 'initializer' do
    should.not.raise(Exception) { Program.new @cxt, [src] }
  end

  the 'build mehtod' do
    p = Program.new @cxt, [src]
    devs =  @cxt.info(CL_CONTEXT_DEVICES)
    should.not.raise(Exception) { p.build devs, "", nil }
    build_log = p.build_info(devs.first, CL_PROGRAM_BUILD_LOG)
    build_log.should.is_a String
  end

  the 'info() method.' do
    p = Program.new @cxt, [src]
    s = p.info(CL_PROGRAM_SOURCE)
    s.should.is_a String
    puts s
  end

  it 'should report build errors.' do
    p = Program.new @cxt, [wrong_src]
    devs = @cxt.info(CL_CONTEXT_DEVICES)
    should.raise(CLError) { p.build devs, "", nil }
    puts p.build_info(devs.first, CL_PROGRAM_BUILD_LOG)
  end
end
