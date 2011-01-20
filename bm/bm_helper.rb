# encoding: utf-8

require 'benchmark'
require File.join(File.dirname(__FILE__), '../lib/opencl')

include Benchmark
include OpenCL

def profile(name, times, &blk)
  begin
    require 'perftools'

    output = "/tmp/#{name}_profile"
    svg_output = "~/Desktop/#{name}_profile.svg"

    PerfTools::CpuProfiler.start(output) do
      times.to_i.times &blk
    end
    `pprof.rb --svg #{output} >> #{svg_output}`
    puts "Check the graphical profiling result in #{svg_output}"
  rescue LoadError
    warn 'The excellent gem "perftools.rb" is not installed.'
  end
end
