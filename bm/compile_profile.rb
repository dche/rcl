# encoding: utf-8

require File.join(File.dirname(__FILE__), 'bm_helper')
require File.join(File.dirname(__FILE__), '../lib/opencl/compiler')

include OpenCL::Compiler

parser = RubyBlockParser.new

profile(:compile, 1) do
  # TODO: use real (and big) kernel sources here.
  block = parser.parse <<-EOK
    gid = get_global_id(0)
  EOK

  src = block.to_cl :kernel, :profile
end
