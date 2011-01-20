# encoding: utf-8

module OpenCL
  class NDArray
    lib = Class.new(Library) do

      type :inexact

      def_map_kernel(:rcl_abs_f, :abs) do |flt|
        "fabs(#{flt})"
      end

      def_map_kernel(:rcl_sin, :sin) do |flt|
        "native_sin(#{flt})"
      end

      def_map_kernel(:rcl_cos, :cos) do |flt|
        "native_cos(#{flt})"
      end

      def_map_kernel(:rcl_sqrt, :sqrt) do |flt|
        "native_sqrt(#{flt})"
      end

      def_map_kernel(:rcl_tan, :tan) do |flt|
        "native_tan(#{flt})"
      end

      def_map_kernel(:rcl_log, :log) do |flt|
        "native_log(#{flt})"
      end

    end

    use lib
  end
end
