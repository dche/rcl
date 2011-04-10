# encoding: utf-8

module OpenCL
  class Bitmap < Operand
    def initialize(size)
      super((size - 1 >> 5) + 1, :cl_uint)
      @capacity = size
      @count = 0
    end

    # How many cells have been marked in the bitmap.
    def count
      return @count if @count >= 0
      @count = self.bits.sum
    end

    def recount
      @count = -1
    end

    # Find next available cell in the bit map, randomly.
    def next_cell
      raise RuntimeError, 'bitmap is full. An resize is expected.' if self.count == @capacity

      i = ::Kernel.rand(self.length)
      return next_cell if i * 32 > @capacity

      n = 0
      bits = self[i]
      while (bits & 0x1) > 0
        n += 1
        bits >>= 1
      end
      n > 31 ? next_cell : i * 32 + n
    end

    def resize
      newcap = @capacity * 2
      return self if newcap < self.byte_size * 8

      super(self.size * 2)
      @capacity = newcap
      self
    end

    # Mark a bit in the bit map.
    #
    def mark_cell(i)
      return self if self.set?(i)
      self[i / 32] |= (1 << (i % 32))
      # ensure the count is updated.
      self.count
      @count += 1
      self
    end

    def set?(i)
      self[i / 32] & (1 << (i % 32)) != 0
    end

    lib = Class.new(Library) do
      type :cl_uint

      # TODO: duplicated with vector/arith.rb.
      #       reanme or generalize it (move to Operand).
      def_reduction_kernel(:rcl_vector_summary, :summary) do |v1, v2|
        "#{v1} + #{v2}"
      end
      alias_method(:sum, :summary)

      def_kernel(:rcl_bitmap_bits) do
        <<-EOK
__kernel void
rcl_bitmap_bits(__global uint *bitmap, int length)
{
    int gid = get_global_id(0);
    if (gid < length) {
        uint bits = bitmap[gid];
        uint n = 0;
        while (bits > 0) {
            if ((bits & 0x1) > 0) n++;
            bits = bits >> 1;
        }
        bitmap[gid] = n;
    }
}
        EOK
      end

      def_method(:bits) do
        execute_kernel :rcl_bitmap_bits, [self.length], :mem, self, :cl_int, self.length
      end
    end

    use lib
  end
end
