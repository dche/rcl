
require 'rubygems'
require 'bacon'

module Bacon
  class Context
    alias :the :it
    alias :its :it    
  end
end
Bacon.extend Bacon::TapOutput

require File.join(File.dirname(__FILE__), '../lib/opencl')
