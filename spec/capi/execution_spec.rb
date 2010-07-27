
require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL
include OpenCL::Capi

cxt = OpenCL::Capi::Context.new(nil, CL_DEVICE_TYPE_ALL)
cq = cxt.create_command_queue

dot_product = <<-EOK
  __kernel void
  dot_product (__global const float4 *a,
               __global const float4 *b,
               __global float *c)
  {
    int gid = get_global_id(0);

    c[gid] = dot(a[gid], b[gid]);
  }
EOK

mult = <<-EOK
  __kernel void
  square (__global const float *in,
          __global float *out,
          float fac)
  {
    int gid = get_global_id(0);
    out[gid] = in[gid] * fac;
  }
EOK

describe "Enqueue kernel" do
  the 'dot product' do
    prog = cxt.create_program(dot_product)
    prog.build cxt.devices, "", nil
    
    k = prog.create_kernels.first
    in_a = HostPointer.new(:cl_float4, 1)
    in_b = HostPointer.new(:cl_float4, 1)
    in_a[0] = [1, 2, 3, 4]
    in_b[0] = [4, 3, 2, 1]

    mem_a = cxt.create_buffer(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 0, in_a)
    mem_b = cxt.create_buffer(CL_MEM_READ_ONLY, in_b.byte_size)
    cq.enqueue_write_buffer(mem_b, true, 0, in_b.byte_size, in_b, nil)

    out_c = HostPointer.new(:cl_float, 1)
    mem_c = cxt.create_buffer(CL_MEM_WRITE_ONLY, out_c.byte_size)
    
    k.set_arg(0, mem_a)
    k.set_arg(1, mem_b)
    k.set_arg(2, mem_c)
    evt = cq.enqueue_NDRange_kernel(k, 1, [in_a.size], nil, nil)
    cq.finish

    out_c[0].should.equal 0
    cq.enqueue_read_buffer(mem_c, true, 0, out_c.byte_size, out_c, nil)
    out_c[0].should.equal 20
  end

  the 'mult' do
    prog = cxt.create_program(mult)
    prog.build cxt.devices, "", nil
    
    k = prog.create_kernels.first
    in_f = HostPointer.new(:cl_float, 4)
    in_f.assign [1, 2, 3, 4]
    
    mem_in = cxt.create_buffer(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 0, in_f)
    out_f = HostPointer.new(:cl_float, 4)
    mem_out = cxt.create_buffer(CL_MEM_WRITE_ONLY, out_f.byte_size)
    
    k.set_arg(0, mem_in)
    k.set_arg(1, mem_out)
    k.set_arg(2, [10.05].pack('f'))
    evt = cq.enqueue_NDRange_kernel(k, 1, [in_f.size], nil, nil)
    cq.finish
    cq.enqueue_read_buffer(mem_out, true, 0, in_f.byte_size, out_f, nil)
    out_f[0].should.close(10.05, 1e-5)
    out_f[1].should.close(20.10, 1e-5)
    out_f[2].should.close(30.15, 1e-5)
    out_f[3].should.close(40.20, 1e-5)
  end
end
