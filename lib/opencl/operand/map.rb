# encoding: utf-8

module OpenCL
  class Library
    class <<self
      # Defines an Operator by providing the source code of a unary operation
      # which takes a single element of the Vecotr as argument.
      #
      # Example
      #
      #   def_map_operator(:rcl_native_sin, :sin, :inexact) do |num|
      #     "native_sin(#{num})"
      #   end
      def def_map_kernel(kernel_name, method_name, &blk)
        tmpl = <<-EOT
__kernel void
#{kernel_name}(const __global T *vec, const unsigned int length, __global T *out)
{
#define UNI_OP(x)  (<%= uni_op %>)

    int gid = get_global_id(0);
    if (gid < length) {
        out[gid] = (T)(UNI_OP(vec[gid]));
    }

#undef UNI_OP
}
        EOT

        uni_op = yield 'x'
        def_kernel(kernel_name) do
          ERB.new(tmpl).result(binding)
        end

        def_method(method_name) do |out = nil|
          self.map kernel_name, out
        end
      end
    end
  end
end
