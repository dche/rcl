# encoding: utf-8

module OpenCL
  class Vector
    lib = Class.new(Library) do
      type :inexact

      def_kernel(:rcl_mad_vv) do
        <<-EOK
__kernel void
rcl_mad_vv(__global T *vec, int length,
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
rcl_mad_vn(__global T *vec, int length, T mul_num, T add_num)
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
          execute_kernel :rcl_mad_vv, [sz], :mem, self, :cl_int, sz, :mem, mval, :mem, aval
        else
          # do not check if aval is a number, the Kernel#set_arg will spot
          # this error finally.
          execute_kernel :rcl_mad_vn, [sz], :mem, self, :cl_int, sz, tag, mval, tag, aval
        end
      end

      def_method(:square) do
        self ** 2
      end

    end
    use lib
  end
end
