# encoding: utf-8
#
# This file defines OpenCL builtin functions. These functions are ciritical
# to type inferring.

module OpenCL
  module Compiler

    class CL < Language

      function(:sin, []);
      function(:cos, [])
    end
  end
end
