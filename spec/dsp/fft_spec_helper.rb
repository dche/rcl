require File.join(File.dirname(__FILE__), '../spec_helper')
require File.join(File.dirname(__FILE__), '../../lib/opencl/dsp/fft')

include OpenCL
include OpenCL::DSP

def valid_fft_shapes(num = 10)
  ptwo = 1.upto(12).map do |n|; 2 ** n; end
  pairs = []
  ptwo.each do |n|; pairs << [n]; pairs << [n, n]; pairs << [n, n, n]; end
  mix2 = ptwo.permutation(2).to_a
  mix3 = ptwo.permutation(3).to_a
  shapes = mix2.shuffle.take(num) + mix3.shuffle.take(num) + pairs.shuffle.take(num) << [1048576]
  
  shapes.delete_if do |shp|
    shp.reduce(:*) * 6 * OpenCL.type_size(:cl_float2) > OpenCL::Context.default_context.max_mem_alloc_size
  end
end
