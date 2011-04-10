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

    start_time = Time.now
    PerfTools::CpuProfiler.start(output) do
      times.to_i.times &blk
    end
    puts "Profiling #{times} times execution finished in #{Time.now - start_time} seconds."
    puts "Processing..."
    `pprof.rb --svg #{output} >> #{svg_output}`
    puts "Done. Check the graphical profiling result in #{svg_output}"
  rescue LoadError
    warn 'The excellent gem "perftools.rb" is needed but not installed.'
  end
end
