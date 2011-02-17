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
    @cxt = OpenCL::Capi::Context.new(platforms.first, CL_DEVICE_TYPE_DEFAULT)
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

  the 'workgroup_info()' do
    k = @prog.create_kernels.first
    lws = k.workgroup_info(@cxt.devices.first, CL_KERNEL_WORK_GROUP_SIZE)
    lws.should.not.equal 0
    puts "CL_KERNEL_WORK_GROUP_SIZE: #{lws}"
    lms = k.workgroup_info(@cxt.devices.first, CL_KERNEL_LOCAL_MEM_SIZE)
    puts "CL_KERNEL_LOCAL_MEM_SIZE: #{lms}"
  end

  it 'should be able to create a kernel more than once' do
    k = @prog.create_kernel 'dot_product'
    k2 = nil
    should.not.raise(Exception) { k2 = @prog.create_kernel 'dot_product' }
    k2.function_name.should.equal k.function_name
  end
end
