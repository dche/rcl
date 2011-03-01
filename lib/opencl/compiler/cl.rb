# encoding: utf-8

require File.join(File.dirname(__FILE__), 'language')

# include Sexp processors for CL language.
require File.join(File.dirname(__FILE__), 'cl/type')
require File.join(File.dirname(__FILE__), 'cl/builtin')
require File.join(File.dirname(__FILE__), 'cl/operator')
require File.join(File.dirname(__FILE__), 'cl/cl')

module OpenCL
  module Compiler
    # A Ruby to OpenCL compiler.
    class CL

      class << self
        # Compiles a block.
        #
        # Returns a Function object.
        def compile(name, block_src, is_kernel = false)
          parser = RubyBlockParser.new
          ast = parser.parse(block_src)
          if ast.nil?
            raise SyntaxError(parser)
          end

          sexp = ast.to_sexp.extend(Sexp)
          args = sexp.args
          body = sexp.block

          symbols = {}
          # add arguments to symbol table
          args.body.each do |arg|
            _, name = arg
            name = name.to_sym
            raise CompileError, "found duplicated argument name #{name}." if symbols.has_key?(name)
            symbols[name] = arg
          end
          lang = self.new(symbols)
          type = lang.infer(body)

          compiled_src = build_header(args, type, lang.symbols, is_kernel) + lang.translate(body)
          Function.new name, type, compiled_src
        end

        private

        def build_header(sexp, return_type, symbols, is_kernel)
          p sexp
          p return_type
          p symbols
          p is_kernel
          "__kernel void\n"
        end
      end

      private


    end
  end
end

