# encoding: utf-8

require File.join(File.dirname(__FILE__), 'grammar')
require File.join(File.dirname(__FILE__), 'sexp')

require File.join(File.dirname(__FILE__), 'type')
require File.join(File.dirname(__FILE__), 'function')

module OpenCL
  module Compiler

    class CompileError < RuntimeError; end
    # Raised when type inferring can't work.
    class TypeError < CompileError; end
    # Raised when referenced function is not found during translation.
    class LinkError < CompileError; end

    # Base class of all target language translators.
    class Language

      def initialize(symbols)
        raise ArgumentError, "expected a Hash, got a #{symbols.class}" unless symbols.is_a?(Hash)
        @symbols = symbols
      end

      def symbols
        @symbols.clone.freeze
      end

      def infer(sexp)
        process sexp, :infer
      end

      def translate(sexp)
        process sexp, :translate
      end

      # Defines a translator.
      #
      # Example
      #
      #  class JavaScript
      #    translate(:+) do |sexp|
      #      lhs, rhs = sexp.body
      #      self.translate(lhs) + '+' + self.translate(rhs)
      #    end
      #  end
      def self.translator(tag, &blk)
        self.handlers(:translate)[tag] = blk
      end

      # Defines a type inferrer.
      #
      # Example
      #
      #  class CL
      #    infer(:) do |sexp|
      #
      #    end
      #  end
      def self.inferrer(tag, &blk)
        self.handlers(:infer)[tag] = blk
      end

      class CompileError < RuntimeError; end
      # Raised when type inferring can't work.
      class TypeError < CompileError; end
      # Raised when referenced function is not found during translation.
      class LinkError < CompileError; end


      # Defines a built-in funciton provided by the language.
      # The major use is to provide type information to facilitate
      # type inferring.
      def self.function(name, type, src = nil)
        @built_in_functions ||= {}
        @built_in_functions[name] = Function.new(name, type, src)
      end

      # Returns the translators or type inferrers of the Language.
      def self.handlers(action)
        case action
        when :infer
          @inferrers ||= {}
        when :translate
          @translators ||= {}
        else
          nil
        end
      end

      private

      def process(sexp, action)
        tag = sexp.tag
        proc = self.class.handlers(action)[tag]
        raise CompileError, "unkown s-exp type: :#{tag}" if proc.nil?

        self.instance_exec sexp, &proc
      end
    end
  end
end

