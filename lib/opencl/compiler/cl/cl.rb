# encoding: utf-8
#
# This file defines OpenCL builtin functions

module OpenCL
  module Compiler
    class CL < Language
      #
      inferrer(:block) do |sexp|
        # the parent sexp should create a temp variable to receive the value of block
        # ask each statements to infer its own type.
        # add the constraint that all exit point (return, break, last statment)
        # shall have same type.
        # set the temp variable's type
        # return the type or type valriable?
      end

      translator(:block) do |sexp|
        src = ''
        sexp.body.each do |stmt|
          # translate the block statements used by the stmt
          # declare temp variable if necessary
          self.translate(stmt)
        end
        src.empty? ? src : "{\n#{src}\n}\n"
      end

      inferrer(:var) do |sexp|
        name = sexp.last
        s = @symbols.has_key?(name)
        return s.atrribute(:type) unless s.nil?


      end

      translator(:var) do |sexp|
        # just returns the variable name.
        sexp.last
      end

      inferrer(:int) do |sexp|
        sexp.attribute(:type, :cl_long)
      end

      translator(:int) do |sexp|
        # just returns the literal.
        sexp.last
      end

      inferrer(:float) do |sexp|
        sexp.attribute(:type, :cl_float)
      end

      translator(:float) do |sexp|
        sexp.last
      end




    end
  end
end
