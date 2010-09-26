
require_relative 'opencl/capi_support'

require_relative 'opencl/cl_error'
require_relative 'opencl/context'
require_relative 'opencl/program'
require_relative 'opencl/buffer'

# A rubyish interface to OpenCL.
#
module OpenCL

  module_function
  
  def type_size(type)
    SCALAR_TYPES[type] || VECTOR_TYPES[type]
  end
  
  def valid_type?(type)
    SCALAR_TYPES.has_key?(type) || VECTOR_TYPES.has_key?(type)
  end

end
