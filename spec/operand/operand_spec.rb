# encoding: utf-8

require File.join(File.dirname(__FILE__), '../spec_helper')

describe Operand do
  the '#resize' do
    op = Operand.new 32, :cl_float
    op[0] = 1.125
    op[1] = 2.125
    op[31] = 3.5
    should.raise(Exception) do
      op[32] = 4.0
    end

    op.resize 32
    op.should.be.pointer_mapped
    op.size.should.equal 32

    op.resize 64
    op.should.not.be.pointer_mapped
    op.size.should.equal 64
    op.byte_size.should.equal 64 * OpenCL.type_size(:cl_float)
    op[0].should.equal 1.125
    op[1].should.equal 2.125
    op[31].should.equal 3.5
    should.not.raise(Exception) do
      op[32] = 4.0
    end
    op[32].should.equal 4.0
  end

  class SpecOperand1 < Operand; end

  class SpecOperand2 < Operand; end

  class SpecLib1 < Library; end

  class SpecLib2 < Library; end

  the "libraries of subclasses should not overlap." do
    SpecOperand1.use SpecLib1
    SpecOperand1.libraries.should.not.empty
    SpecOperand2.libraries.should.be.empty
    SpecOperand2.use SpecLib2
    SpecOperand1.libraries.length.should.equal 1
    SpecOperand2.libraries.length.should.equal 1
  end

  the '#clear' do
    op = Operand.new 2, :cl_uint
    op[0] = 1
    op[1] = 2047
    op[1].should.equal 2047
    op.clear
    op[0].should.equal 0
    op[1].should.equal 0
  end
end
