# encoding: utf-8

require File.join(File.dirname(__FILE__), 'bm_helper')
require File.join(File.dirname(__FILE__), '../lib/opencl/vector')

begin
require 'perftools'

vec = Vector.rand 1_000_000

PerfTools::CpuProfiler.start("/tmp/reduce_profile") do
  1000.times do
    vec.sum
  end
end

rescue LoadError
  puts 'perftools.rb is not installed.'
end
