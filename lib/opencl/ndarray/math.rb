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

      def_kernel(:rcl_power) do
        <<-EOK
__kernel void
rcl_power(__global T *vec, int length, T number)
{
    int gid = get_global_id(0);
    if (gid < length) {
        vec[gid] = pow(vec[gid], number);
    }
}
        EOK
      end

      def_method(:power) do |number|
        sz = self.size
        execute_kernel :rcl_power, [sz], :mem, self, :cl_int, sz, self.type.tag, number
      end
      alias_method :**, :power

    end

    use lib
  end
end
