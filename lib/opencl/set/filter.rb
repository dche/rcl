# encoding: utf-8

module OpenCL
  class SetLibrary < Library
    def self.def_map_kernel(kernel_name, method_name, &blk)
      tmpl = <<-EOT
__kernel void
#{kernel_name}(__global T *set, int length, __global uint *bitmap)
{
#define MAP_OP(elm, keep, dirty) \\
do { \\
<%= map_op %> \\
} while (0)

    int gid = get_global_id(0);
    // the global size is the capacity of the bitmpa of the Set.
    if (gid < length) {
        // the first 32-bits of Bitmap is reserved.
        uint bits = bitmap[gid + 1];
        uint clean_bits = bits;

        for (int i = 0; i < 32; i++) {
            if (rcl_set_test_bit(bits, i)) {
                bool keep = true;
                bool dirty = false;

                int index = (gid * 32 + i) + 1;
                T elm = set[index];

                MAP_OP(elm, keep, dirty);
                if (!keep) {
                    clean_bits = rcl_set_clear_bit(clean_bits, i);
                }
                if (dirty) {
                    set[index] = elm;
                }
            }
        }
        if (bits != clean_bits) {
            bitmap[gid + 1] = clean_bits;
        }
    }
#undef MAP_OP
}
      EOT

      map_op = yield('elm', 'keep', 'dirty').lines.map(&:chomp).join(" \\\n")
      def_kernel(kernel_name) do
        ERB.new(tmpl).result(binding)
      end

      def_method(method_name) do
        self.map(kernel_name)
      end
    end

    def self.def_gc_kernel(&blk)
      def_map_kernel(:rcl_set_gc, :gc) do |elm, keep, dirty|
        zero_op = yield(elm);

        <<-EOK
if (#{zero_op}) {
    #{keep} = false;
}
        EOK
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
      dim = @bitmap.length
      execute_kernel kernel_name, [dim], :mem, self, :cl_int, dim, :mem, @bitmap
      @bitmap.recount
    end

    lib = Class.new(SetLibrary) do
      type :any

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
