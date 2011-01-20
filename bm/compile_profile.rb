# encoding: utf-8

require File.join(File.dirname(__FILE__), 'bm_helper')
require File.join(File.dirname(__FILE__), '../lib/opencl/compiler/grammar')

include OpenCL::Compiler

parser = RubyBlockParser.new

profile(:compile, 100) do
  # TODO: use real (and big) kernel sources here.
  parser.parse <<-EOK
    gid = get_global_id(0)
  EOK
end
