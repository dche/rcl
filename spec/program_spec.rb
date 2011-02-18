# encoding: utf-8

require File.join(File.dirname(__FILE__), 'spec_helper')

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
    # NOTE: NVIDIA's OpenCL impl uses clang to compile the program, which
    #       just quits the whole process when compile opetion is wring.
#    should.raise(CLError) { OpenCL::Program.new valid_src, '-cl-ruby' }
  end

  it 'should be able to change program source' do
    prog = OpenCL::Program.new valid_src
    prog.source.should.equal valid_src
    prog.compile multi_kernel_src
    prog.source.should.equal multi_kernel_src
  end

  the '#kernel' do

    prog = OpenCL::Program.new valid_src

    should.raise(OpenCL::Capi::CLError) {
      prog.kernel('no_such_kernel')
    }

    prog.kernel(:dot_product).should.is_a OpenCL::Capi::Kernel
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

  the '#call with block' do

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

  the 'method_missing' do
    prog = OpenCL::Program.new multi_kernel_src

    p1 = HostPointer.new :cl_float, 4
    p1.assign [1, 2, 3, 4]
    p1[2].should.equal 3

    m1 = Buffer.new p1.byte_size, :in
    m1.write p1
    m2 = Buffer.new p1.byte_size, :out
    should.raise(NoMethodError) { prog.add [2] }
    should.not.raise(NoMethodError) {
      prog.mult([4], :mem, m1, :mem, m2, :cl_float, 10)
    }
    m2.read p1
    p1[0].should.equal 10
    p1[1].should.equal 20
    p1[2].should.equal 30
    p1[3].should.equal 40
  end

  the 'Profiling' do
    prog = OpenCL::Program.new multi_kernel_src
    prog.profiling?.should.be.false
    prog.execution_time.should.equal 0
    prog.profiling = true
    prog.execution_time.should.equal 0
    prog.profiling?.should.be.true

    byte_size = 4 * OpenCL.type_size(:cl_float)
    m1 = Buffer.new byte_size, :in
    m2 = Buffer.new byte_size, :out
    prog.mult([4], :mem, m1, :mem, m2, :cl_float, 100)
    et = prog.execution_time
    (et > 0).should.be.true

    prog.profiling = false
    prog.should.not.be.profiling
    prog.execution_time.should.equal 0
  end
end
