
require_relative '../spec_helper'
require_relative '../../lib/opencl/string'

describe String do
  
  it 'should complain if the module name is invalid.' do
    should.raise(ArgumentError) {
      "".rcl_include 'no_such_module'
    }
  end
  
  it 'should prepend contents of the included module to itself.' do
    ''.rcl_include('random').should.not.be.empty
  end

end

describe OpenCL do
  the '#utility_modules' do
    OpenCL.utility_modules.should.include('random')
  end
end
