# encoding: utf-8

module OpenCL
  class Vector
    lib = Class.new(Library) do

      type :inexact

      def_map_kernel(:rcl_abs_v_f, :abs) do |item|
        "fabs(#{item})"
      end

      def_kernel(:rcl_mad_vv) do
        <<-EOK
__kernel void
rcl_mad_vv(__global T *vec, uint length,
           __global const T *mul_vec,
           __global const T *add_vec)
{
    int gid = get_global_id(0);
    if (gid < length) {
        vec[gid] = mad(vec[gid], mul_vec[gid], add_vec[gid]);
    }
}
        EOK
      end

      def_kernel(:rcl_mad_vn) do
        <<-EOK
__kernel void
rcl_mad_vn(__global T *vec, uint length, T mul_num, T add_num)
{
    int gid = get_global_id(0);
    if (gid < length) {
        vec[gid] = mad(vec[gid], mul_num, add_num);
    }
}
        EOK
      end

      def_method(:mad) do |mval, aval|
        tag = self.type.tag
        sz = self.size

        if Vector === mval
          unless self.type_compatible?(mval) && self.type_compatible?(aval)
            raise ArgumentError, "incompatible Vectors"
          end
          execute_kernel :rcl_mad_vv, [sz], :mem, self, :cl_uint, sz, :mem, mval, :mem, aval
        else
          # do not check if aval is a number, the Kernel#set_arg will spot
          # this error finally.
          execute_kernel :rcl_mad_vn, [sz], :mem, self, :cl_uint, sz, tag, mval, tag, aval
        end
      end

      def_kernel(:rcl_power_vn) do
        <<-EOK
__kernel void
rcl_power_vn(__global T *vec, uint length, T number)
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
        execute_kernel :rcl_power_vn, [sz], :mem, self, :cl_uint, sz, self.type.tag, number
      end
      alias_method :**, :power

      def_method(:square) do
        self ** 2
      end

      def_map_kernel(:rcl_vector_sin, :sin) do |flt|
        "native_sin(#{flt})"
      end

      def_map_kernel(:rcl_vector_cos, :cos) do |flt|
        "native_cos(#{flt})"
      end

      def_map_kernel(:rcl_vector_sqrt, :sqrt) do |flt|
        "native_sqrt(#{flt})"
      end

      def_map_kernel(:rcl_vector_tan, :tan) do |flt|
        "native_tan(#{flt})"
      end

      def_map_kernel(:rcl_vector_log, :log) do |flt|
        "native_log(#{flt})"
      end

    end

    use lib
  end
end