
require File.join(File.dirname(__FILE__), 'bm_helper')
require File.join(File.dirname(__FILE__), '../lib/opencl/vector')

vec = Vector.ones 1_000_000

profile(:reduce, 1_000) do
  vec.sum
end
