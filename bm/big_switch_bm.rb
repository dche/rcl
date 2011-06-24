# encoding: utf-8

require File.join(File.dirname(__FILE__), 'bm_helper')
require File.join(File.dirname(__FILE__), '../lib/opencl/vector')

a = [1.0, 2.0]

class MyVector < Vector
  lib = Class.new(Library) do
    type :cl_float

    # a very very inefficient kernel.
    def_kernel(:rcl_bm_big_switch) do
      <<-EOK
__kernel void
rcl_bm_big_switch(__global T *vec, const int length)
{
    int gid = get_global_id(0);
    if (gid < length) {
        T num = vec[gid];
        int gm = gid % 20;
        switch(gm) {
        case 0:
          vec[gid] = num  + gm;
          break;
        case 1:
          vec[gid] = num  + gm;
          break;
        case 2:
          vec[gid] = num  + gm;
          break;
        case 3:
          vec[gid] = num  + gm;
          break;
        case 4:
          vec[gid] = num  + gm;
          break;
        case 5:
          vec[gid] = num  + gm;
          break;
        case 6:
          vec[gid] = num  + gm;
          break;
        case 7:
          vec[gid] = num  + gm;
          break;
        case 8:
          vec[gid] = num  + gm;
          break;
        case 9:
          vec[gid] = num  + gm;
          break;
        case 10:
          vec[gid] = num  + gm;
          break;
        case 11:
          vec[gid] = num  + gm;
          break;
        case 12:
          vec[gid] = num  + gm;
          break;
        case 13:
          vec[gid] = num  + gm;
          break;
        case 14:
          vec[gid] = num  + gm;
          break;
        case 15:
          vec[gid] = num  + gm;
          break;
        case 16:
          vec[gid] = num  + gm;
          break;
        case 17:
          vec[gid] = num  + gm;
          break;
        case 18:
          vec[gid] = num  + gm;
          break;
        case 19:
          vec[gid] = num  + gm;
          break;
        }
    }
}
      EOK
    end

    def_method(:bm_big_switch) do
      execute_kernel :rcl_bm_big_switch, [self.length], :mem, self, :cl_int, self.length
    end
  end

  use lib
end

vec = MyVector.ones 1_000_000

bm(8) do |b|
  b.report('Ruby') do
    1_000_000.times { c = a[0] * a[1] }
  end

  b.report('OpenCL') do
    2.times {
      vec.bm_big_switch
    }
  end
end
