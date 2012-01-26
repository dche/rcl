# encoding: utf-8

module OpenCL
  # Bitmap is a "map" operation friendly data structure.
  #
  class Bitmap < Operand
    def initialize(size, allocation_strategy = :random)
      super(size_for_cap(size), :cl_uint)

      @capacity = size
      @count = 0
      @alloc_meth = allocation_strategy == :random ? :next_random_cell : :next_linear_cell
      # used by :linear allocation to know whare to start scan next free cell.
      @next_cell = 0
    end

    # How many cells have been marked in the bitmap.
    attr_reader :count
    # How many bits the receiver can hold.
    attr_reader :capacity

    # Re-calculate the count.
    def recount
      @count = 0
      (self.length - 1).times do |n|
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
      self.send(@alloc_meth)
    end

    # Increases the capacity.
    def resize(sz)
      return self if sz <= @capacity

      @capacity = sz
      super size_for_cap(@capacity)
      self
    end

    # Marks a bit in the bit map.
    def mark_cell(i)
      return self unless in_range?(i)
      return self if self.set?(i)

      d, m = i.divmod 32
      self[d] = self[d] | (1 << m)
      @count += 1

      if @alloc != :random
        # pi == @next_cell
        # new @next_cell needs not to be available.
        @next_cell = (i == @capacity - 1) ? 0 : i + 1
      end
      self
    end

    # Clear a bit in the receiver.
    def clear_cell(i)
      return self unless self.set?(i)

      d, m = i.divmod 32
      self[d] = self[d] & ~(1 << m)
      @count -= 1
      self
    end

    # Returns +true+ if a bit is set in the receiver.
    def set?(i)
      return false unless in_range?(i)

      d, m = i.divmod 32
      self[d] & (1 << m) != 0
    end

    protected

    def [](i)
      super i + 1
    end

    def []=(i, v)
      super i + 1, v
    end

    private

    def size_for_cap(cap)
      (cap - 1 >> 5) + 2
    end

    def in_range?(i)
      i >= 0 && i < @capacity
    end

    def next_random_cell
      i = ::Kernel.rand(self.length - 1)
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
      i, n = @next_cell.divmod 32

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
