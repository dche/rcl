# encoding: utf-8

module OpenCL
  # Bitmap is a "map" operation friendly data structure.
  class Bitmap < Operand
    def initialize(size, allocation_strategy = :random)
      super((size - 1 >> 5) + 1, :cl_uint)

      @capacity = size
      @count = 0
      @alloc = allocation_strategy
      @next_cell = 0
    end

    # How many cells have been marked in the bitmap.
    attr_reader :count
    # How many bits the receiver can hold.
    attr_reader :capacity

    # Re-calculate the count.
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

    # Finds next available cell in the bit map.
    #
    # Returns the index of hte cell.
    def next_cell
      raise RuntimeError, 'bitmap is full. An resize is expected.' if self.count == @capacity
      @alloc == :rancom ? next_random_cell : next_linear_cell
    end

    # Increases the capacity.
    def resize(sz)
      @capacity = sz if sz > 0
      return self if sz < self.byte_size * 8

      super (sz - 1 >> 5) + 1
      self
    end

    # Marks a bit in the bit map.
    def mark_cell(i)
      return self if self.set?(i)

      self[i / 32] |= (1 << (i % 32))
      @count += 1

      if @alloc != :random
        # i == @next_cell
        @next_cell = (i == @capacity - 1) ? 0 : i + 1
      end
      self
    end

    # Returns +true+ if a bit is set in the receiver.
    def set?(i)
      self[i >> 5] & (1 << (i % 32)) != 0
    end

    private

    def next_random_cell
      i = ::Kernel.rand(self.length)
      return next_random_cell if i * 32 > @capacity

      n = 0
      bits = self[i]
      while (bits & 0x1) > 0
        n += 1
        bits >>= 1
      end
      n > 31 ? next_random_cell : i * 32 + n
    end

    def next_linear_cell
      i = @next_cell >> 5
      n = @next_cell % 32

      bits = (self[i] >> n)
      b = n
      while (bits & 0x1) > 0
        b += 1
        bits >>= 1
      end
      @next_cell += (b - n)

      if b > 31
        @next_cell %= @capacity
        next_linear_cell
      else
        @next_cell
      end
    end
  end
end
