# encoding: utf-8

require 'treetop'

require File.join(File.dirname(__FILE__), 'grammar/syntax_node')

Treetop.load File.join(File.dirname(__FILE__), 'grammar/tokenizer')
Treetop.load File.join(File.dirname(__FILE__), 'grammar/number')
Treetop.load File.join(File.dirname(__FILE__), 'grammar/identifier')
Treetop.load File.join(File.dirname(__FILE__), 'grammar/expression')
Treetop.load File.join(File.dirname(__FILE__), 'grammar/block')

module OpenCL
  module Compiler
    # Reports errors found by parser.
    class ParseError < RuntimeError
      attr_reader :line, :column

      def initialize(parser)
        @line = parser.failure_line
        @column = parser.failure_column

        msg = "Syntax error: line: #{@line}, column: #{@column}\n" # parser.failure_reason

        mark = '~' * (@column - 1) + '^'

        context = 3 # how many lines to show.
        sl = [@line - context, 0].max
        src = parser.input.lines.to_a[sl, context * 2].map(&:chomp)
        src.insert([@line, context].min, mark)

        super(msg + src.join("\n"))
      end
    end
  end
end

