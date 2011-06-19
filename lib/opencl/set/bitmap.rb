# encoding: utf-8

module OpenCL
  # Bitmap is a "map" operation friendly data structure.
  class Bitmap < Operand
    def initialize(size)
      super((size - 1 >> 5) + 1, :cl_uint)
      @capacity = size
      @count = 0
    end

    # How many cells have been marked in the bitmap.
    attr_reader :count

    def recount
      @count = 0
      self.length.times do |n|
        u32 = self[n]
        32.times do |nb|
          bit = u32 & (1 << nb)
          @count += 1 if bit > 0
        end
      end
      @count
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

    def resize(sz)
      @capacity = sz if sz > 0
      return self if sz < self.byte_size * 8

      super (sz - 1 >> 5) + 1
      self
    end

    # Mark a bit in the bit map.
    #
    def mark_cell(i)
      return self if self.set?(i)

      self[i / 32] |= (1 << (i % 32))
      @count += 1

      self
    end

    def set?(i)
      self[i / 32] & (1 << (i % 32)) != 0
    end

    lib = Class.new(Library) do
      type :cl_uint

    end

    use lib
  end
end
