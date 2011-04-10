# encoding: utf-8

require 'rubygems'

def coveragable?
  RUBY_ENGINE == 'ruby' && RUBY_VERSION >= '1.9'
end

if coveragable?
  require 'coverage'

  at_exit do
    Coverage.result
  end
end

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

Coverage.start if coveragable?

require File.join(File.dirname(__FILE__), '../lib/opencl')
include OpenCL
