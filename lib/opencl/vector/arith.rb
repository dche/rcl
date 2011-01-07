# encoding: utf-8

module OpenCL
  class Vector
    lib = Class.new(Library) do

      type :number

      def_reduction_kernel(:rcl_vector_summary, :summary) do |v1, v2|
        "#{v1} + #{v2}"
      end
      alias_method(:sum, :summary)

      def_kernel(:rcl_add_vv) do
        <<-EOK
__kernel void
rcl_add_vv(__global T *vec, uint length, __global const T *other)
{
    int gid = get_global_id(0);
    if (gid < length) {
        vec[gid] += other[gid];
    }
}
        EOK
      end

      def_kernel(:rcl_add_vn) do
        <<-EOK
__kernel void
rcl_add_vn(__global T *vec, uint length, T number)
{
    int gid = get_global_id(0);
    if (gid < length) {
        vec[gid] += number;
    }
}
        EOK
      end

      def_method(:add) do |val|
        tag = self.type.tag
        sz = self.size
        if Vector === val
          raise ArgumentError, "incompatible Vectors" unless self.type_compatible?(val)
          execute_kernel :rcl_add_vv, [sz], :mem, self, :cl_uint, sz, :mem, val
        else
          execute_kernel :rcl_add_vn, [sz], :mem, self, :cl_uint, sz, tag, val
        end
        self
      end
      alias_method :+, :add

      def_kernel(:rcl_sub_vv) do
        <<-EOK
__kernel void
rcl_sub_vv(__global T *vec, const uint length, __global const T *other)
{
    int gid = get_global_id(0);
    if (gid < length) {
        vec[gid] -= other[gid];
    }
}
        EOK
      end

      def_kernel(:rcl_sub_vn) do
        <<-EOK
__kernel void
rcl_sub_vn(__global T *vec, const uint length, const T number)
{
    int gid = get_global_id(0);
    if (gid < length) {
        vec[gid] -= number;
    }
}
        EOK
      end

      def_method(:sub) do |val|
        tag = self.type.tag
        sz = self.size
        if Vector === val
          raise ArgumentError, "incompatible Vectors" unless self.type_compatible?(val)
          execute_kernel :rcl_sub_vv, [sz], :mem, self, :cl_uint, sz, :mem, val
        else
          execute_kernel :rcl_sub_vn, [sz], :mem, self, :cl_uint, sz, tag, val
        end
        self
      end
      alias_method :-, :sub

      def_kernel(:rcl_mul_vv) do
        <<-EOK
__kernel void
rcl_mul_vv(__global T *vec, uint length, __global const T *other)
{
    int gid = get_global_id(0);
    if (gid < length) {
        vec[gid] *= other[gid];
    }
}
        EOK
      end

      def_kernel(:rcl_mul_vn) do
        <<-EOK
        __kernel void
rcl_mul_vn(__global T *vec, uint length, T number)
{
    int gid = get_global_id(0);
    if (gid < length) {
        vec[gid] *= number;
    }
}
        EOK
      end

      def_method(:mul) do |val|
        tag = self.type.tag
        sz = self.size
        if Vector === val
          raise ArgumentError, "incompatible Vectors" unless self.type_compatible?(val)
          execute_kernel :rcl_mul_vv, [sz], :mem, self, :cl_uint, sz, :mem, val
        else
          execute_kernel :rcl_mul_vn, [sz], :mem, self, :cl_uint, sz, tag, val
        end
        self
      end
      alias_method :*, :mul

      def_kernel(:rcl_div_vn) do
        <<-EOK
__kernel void
rcl_div_vn(__global T *vec, uint length, T number)
{
    int gid = get_global_id(0);

    if (gid < length) {
        if (number < FLT_EPSILON)
          vec[gid] = INFINITY;
        else
          vec[gid] /= number;
    }
}
        EOK
      end

      def_method(:div) do |number|
        sz = self.size
        execute_kernel :rcl_div_vn, [sz], :mem, self, :cl_uint, sz, self.type.tag, number
      end
      alias_method :/, :div

      def_kernel(:rcl_mod_vn) do
        <<-EOK
__kernel void
rcl_mod_vn(__global T *vec, uint length, T number)
{
    int gid = get_global_id(0);

    if (gid < length) {
      <% if self.type.exact? %>
        vec[gid] %= number;
      <% else %>
        vec[gid] = fmod(vec[gid], number);
      <% end %>
    }
}
        EOK
      end

      def_method(:mod) do |number|
        sz = self.size
        execute_kernel :rcl_mod_vn, [sz], :mem, self, :cl_uint, sz, self.type.tag, number
      end
      alias_method :%, :mod

      def_reduction_kernel(:rcl_vector_min, :min) do |v1, v2|
        "min(#{v1}, #{v2})"
      end

      def_reduction_kernel(:rcl_vector_max, :max) do |v1, v2|
        "max(#{v1}, #{v2})"
      end

    end

    use lib
  end
end
