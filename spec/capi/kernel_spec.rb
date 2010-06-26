require File.join(File.dirname(__FILE__), '../spec_helper')

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


describe Kernel do
  before do
    @cxt = OpenCL::Capi::Context.new(nil, CL_DEVICE_TYPE_ALL)
    @devs = @cxt.info(CL_CONTEXT_DEVICES)
    @prog = Program.new(@cxt, [src])
    @prog.build(@devs, "", nil)
  end
  
  the 'constructor' do
    should.not.raise(Exception) { OpenCL::Capi::Kernel.new(@prog, "dot_product") }
  end
  
  the 'info() method' do
    kernels = @prog.create_kernels
    kernels.length.should.equal 1
    k = kernels.first
    k.info(CL_KERNEL_FUNCTION_NAME).should.equal 'dot_product'
    k.info(CL_KERNEL_NUM_ARGS).should.equal 3
  end
end
