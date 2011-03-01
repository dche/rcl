# encoding: utf-8

module OpenCL
  module Compiler
    class BlockNode < Treetop::Runtime::SyntaxNode
      def to_sexp
        [:block] + stmts.to_sexp
      end
    end
  end
end

