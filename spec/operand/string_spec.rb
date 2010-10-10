
require File.join(File.dirname(__FILE__), '../spec_helper')
require File.join(File.dirname(__FILE__), '../../lib/opencl/operand/string')

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
