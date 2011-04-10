# encoding: utf-8

module OpenCL
  class SetLibrary < Library
    def self.def_map_kernel(kernel_name, method_name, &blk)
      tmpl = <<-EOT
__kernel void
#{kernel_name}(__global T *set, int length, __global uint *bitmap)
{
#define MAP_OP(elm, keep) \\
do { \\
<%= map_op %> \\
} while (0)

    int gid = get_global_id(0);
    // the global size is the capacity of the Set.
    if (gid < length) {
        int bit_num = gid / 32;
        int bit_idx = gid % 32;
        uint bits = bitmap[bit_num];

        if (rcl_set_test_bit(bits, bit_idx)) {
            bool keep = true;
            MAP_OP(set[gid], keep);
            if (!keep) {
                bitmap[bit_num] = rcl_set_clear_bit(bits, bit_idx);
            }
        }
    }
#undef MAP_OP
}
      EOT

      map_op = yield('elm', 'keep').lines.map(&:chomp).join(" \\\n")
      def_kernel(kernel_name) do
        ERB.new(tmpl).result(binding)
      end

      def_method(method_name) do
        self.map(kernel_name)
      end
    end
  end

  class Set
    # Restricts Set classes use SetLibraries only.
    def self.use(lib)
      raise TypeError, 'expected a SetLibrary.' unless lib.ancestors.include?(SetLibrary)
      super
    end

    undef :reduce

    def map(kernel_name)
      execute_kernel kernel_name, [self.length], :mem, self, :cl_int, self.length, :mem, @bitmap
      @bitmap.recount
    end

    lib = Class.new(SetLibrary) do
      type :structure

      def_function(:rcl_set_test_bit) do
        <<-EOF
static bool
rcl_set_test_bit(uint bits, int bit_idx)
{
    return ((bits & (0x1 << bit_idx)) != 0) ? true : false;
}
        EOF
      end

      def_function(:rcl_set_clear_bit) do
        <<-EOF
static uint
rcl_set_clear_bit(uint bits, int bit_idx)
{
    return bits & ~(0x1 << bit_idx);
}
        EOF
      end
    end

    use lib
  end
end
