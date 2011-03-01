# encoding: utf-8

module OpenCL
  module Compiler
    class CL < Language

      inferrer(:+) do |sexp|
        lhs, rhs = sexp.body

      end

      translator(:+) do |sexp|
        lhs, rhs = sexp.body

      end

      inferrer(:neg) do |sexp|
        s = sexp.body.first

      end

      translator(:neg) do |sexp|

      end

      private

      def translate_binaray_operator(sexp)
        lhs, rhs = sexp.body

      end

    end
  end
end
