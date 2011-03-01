# encoding: utf-8

require File.join(File.dirname(__FILE__), 'compile_spec_helper')

describe CL do
  CL.compile :test, "1 + 1"
end
