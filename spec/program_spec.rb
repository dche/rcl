
require File.join(File.dirname(__FILE__), 'spec_helper')

include OpenCL

valid_src = <<-EOK
  __kernel void
  dot_product (__global const float4 *a,
               __global const float4 *b,
               __global float *c)
  {
    int gid = get_global_id(0);

    c[gid] = dot(a[gid], b[gid]);
  }
EOK

invalid_src = <<-EOK
  __kernel void
  dot_product (__global const float4 *a,
               __global const float4 *b,
               __global float *c)
  {
    int gid = get_global_id(0)

    c[gid] = dot(a[gid], b[gid]);
  }
EOK

multi_kernel_src = <<-EOK
  __kernel void
  dot_product (__global const float4 *a,
               __global const float4 *b,
               __global float *c)
  {
    int gid = get_global_id(0);

    c[gid] = dot(a[gid], b[gid]);
  }
  __kernel void
  mult (__global const float *in,
          __global float *out,
          float fac)
  {
    int gid = get_global_id(0);
    out[gid] = in[gid] * fac;
  }
EOK

describe Program do
  
  it 'should reject invalid source' do
    should.raise(ProgramBuildError) { OpenCL::Program.new invalid_src }
  end
  
  it 'should reject invalid compile options' do
    should.not.raise(Exception) { OpenCL::Program.new valid_src, '-cl-fast-relaxed-math' }
    should.raise(CLError) { OpenCL::Program.new valid_src, '-cl-ruby' }
  end
  
  it 'should be able to change program source' do
    prog = OpenCL::Program.new valid_src
    prog.source.should.equal valid_src
    prog.compile multi_kernel_src
    prog.source.should.equal multi_kernel_src
  end
  
  the '#call' do
    prog = OpenCL::Program.new valid_src
    ptr = HostPointer.new :cl_float4, 2
    ptr[0] = [1, 2, 3, 4]
    ptr[1] = [4, 3, 2, 1]
    
    m1 = Buffer.new ptr.byte_size, :in
    m1.write ptr
    
    ptr[0] = [5, 6, 7, 8]
    ptr[1] = [8, 7, 6, 5]
    m2 = Buffer.new ptr.byte_size, :in
    m2.write ptr
    
    pres = HostPointer.new :cl_float, 2
    m3 = Buffer.new pres.byte_size, :out
    
    prog.call(:dot_product, [2], :mem, m1, :mem, m2, :mem, m3)
    m3.read pres
    pres[0].should.equal 70
    pres[0].should.equal pres[1]
  end
  
  the 'multiple kernels should work' do
    prog = nil
    should.not.raise(Exception) { prog = OpenCL::Program.new multi_kernel_src }
    
    p1 = HostPointer.new :cl_float, 4
    p1.assign [1, 2, 3, 4].pack('f4')
    p1[2].should.equal 3
    
    m1 = Buffer.new p1.byte_size, :in
    m1.write p1
    m2 = Buffer.new p1.byte_size, :out
    prog.call(:mult, [4], :mem, m1, :mem, m2, :cl_float, 10)
    m2.read p1
    p1[0].should.equal 10
    p1[1].should.equal 20
    p1[2].should.equal 30
    p1[3].should.equal 40
    
    prog.call(:mult, [4], :mem, m1, :mem, m2, :cl_float, 100)
    m2.read p1
    p1[0].should.equal 100
    p1[1].should.equal 200
    p1[2].should.equal 300
    p1[3].should.equal 400
  end
  
  the '#max_workgroup_size' do
    prog = OpenCL::Program.new multi_kernel_src
    (prog.max_workgroup_size > 0).should.be.true
  end
  
  the 'method_missing' do
    prog = OpenCL::Program.new multi_kernel_src
    
    p1 = HostPointer.new :cl_float, 4
    p1.assign [1, 2, 3, 4]
    p1[2].should.equal 3
    
    m1 = Buffer.new p1.byte_size, :in
    m1.write p1
    m2 = Buffer.new p1.byte_size, :out
    should.raise(NoMethodError) { prog.add }
    should.not.raise(NoMethodError) { 
      prog.mult([4], :mem, m1, :mem, m2, :cl_float, 10)
    }
    m2.read p1
    p1[0].should.equal 10
    p1[1].should.equal 20
    p1[2].should.equal 30
    p1[3].should.equal 40
  end
end
