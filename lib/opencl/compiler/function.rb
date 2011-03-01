# encoding: utf-8

module OpenCL
  module Compiler
    # A Function object is the product of compiling a ruby block to OpenCL
    # language. It contains the source code that can be used to compose a
    # program.
    #
    # The type of the Function object might not be fully determined until
    # composition. A Function object is like a function template in C++.
    # +Function#specialize(type)+ method is used to get the final version
    # of the function for certain data type.
    #
    # See OpenCL::Compiler::Type
    class Function
      attr_reader :name

      def initialize(name, type, src)
        @name = name
        @definition = src
        @type = type
      end

      def specialize(type)
        # check if the type is compatible.
        # ask the Type class for a typedef of type.
        # replace the Type template and return
        @definition
      end

      def return_type
      end

      def arg_type(pos)
      end

      private

    end
  end
end
