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
end
