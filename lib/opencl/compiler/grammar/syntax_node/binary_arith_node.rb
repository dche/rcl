# encoding: utf-8

module OpenCL
  module Compiler
    class BinaryArithExressionNode < Treetop::Runtime::SyntaxNode
      def to_sexp
        sexp = lhs.to_sexp
        return sexp if rhs.empty?

        op = rhs.elements.first.op.text_value
        rhs.elements.each do |elm|
          sexp = [op.to_sym, sexp, elm.rhs.to_sexp]
        end
        sexp
      end
    end
  end
end
