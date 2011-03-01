# encoding: utf-8

module OpenCL
  module Compiler
    # A Type object is a data type variable that can hold a type or type
    # class.
    # The type system is a tree of types and type classes.
    class Type

      def initialize(tag = :any)
        @tag = tag
      end

      attr_reader :tag


    end
  end
end
