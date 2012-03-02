# encoding: utf-8

require File.join(File.dirname(__FILE__), 'capi')

module OpenCL
  module Capi
    VERSION = if defined? ::OpenCL::CL_VERSION_1_1
      '1.1'
    else
      '1.0'
    end
  end
end

require File.join(File.dirname(__FILE__), 'capi_support')
if OpenCL::Capi::VERSION > '1.0'
  require File.join(File.dirname(__FILE__), 'capi_support_1_1')
end
