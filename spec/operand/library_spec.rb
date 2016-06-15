
require File.join(File.dirname(__FILE__), '../spec_helper')

class ArithLibrary < Library
  type :number

  def_kernel(:rcl_test_lib_add) do
    <<-EOT
__kernel void
rcl_lib_add(__global T *operand, unsigned int length, T number)
{
    int gid = get_global_id(0);
    if (gid < length) {
        operand[gid] += number;
    }
}
    EOT
  end

  def_method(:test_arith_add) do |number|
    execute_kernel :rcl_test_lib_add, [self.length], :mem, self, :cl_uint, self.length, self.type, number
  end

  alias_method :not_a_test, :test_arith_add
end

class SetLib < Library
  type :structure

  def_kernel(:rcl_lib_join) do
    "I can join sets."
  end

  def_method(:join) do |set|
    self
  end
end

describe Library do

  it 'should have library methods after use_library' do
    o = Operand.new 1024, :cl_float
    o.should.not.respond_to :test_arith_add
    Operand.use ArithLibrary
    o.should.respond_to :test_arith_add
  end

  it 'should not be able to use library for in-compatible type.' do
    Operand.use SetLib
    o = Operand.new 1024, :cl_uchar4
    o.should.not.respond_to :join
    o.should.respond_to :test_arith_add
  end

  it 'should be OK to use a Library that have been used.' do
    should.not.raise(Exception) {
      Operand.use ArithLibrary
      Operand.use ArithLibrary
    }
  end

  it 'should respond to aliased methods' do
    Operand.new(4, :cl_uchar).should.respond_to :not_a_test
  end
end
