
require 'rubygems'
require 'bacon'

module Bacon
  class Context
    alias :the :it
    alias :its :it

    alias :given :describe
    alias :expect :it
  end
end
Bacon.extend Bacon::TapOutput

begin
  require 'simplecov'

  SimpleCov.start do
    project_name "rcl"
    coverage_dir "cov"
    root "#{pwd}/.."
    use_merging true
  end
rescue LoadError
end


require File.join(File.dirname(__FILE__), '../lib/opencl')
include OpenCL
