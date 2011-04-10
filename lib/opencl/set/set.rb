# encoding: utf-8

module OpenCL
  # Set is a pool of comparable elements.
  class Set < Operand

    def initialize(type, size_hint = 1024)
      sz = size_hint < 1024 ? 1024 : next_pow2(size_hint.to_i)
      super sz, type

      @bitmap = Bitmap.new sz
    end

    def initialize_copy(other)
      super
      @bitmap = @bitmap.dup
    end

    # Number of elements in the receiver.
    def count
      @bitmap.count
    end

    # Put an element to the receiver.
    #
    # Returns the index.
    def put(elm)
      resize

      i = @bitmap.next_cell
      begin
        self[i] = elm
        @bitmap.mark_cell(i)
      rescue Exception => e
        raise e
      end
      i
    end

    def [](idx)
      return nil unless @bitmap.set?(idx)
      super
    end

    # Returns true if the receiver fully contains the given object, which can
    # be an element or another Set.
    def include?(obj)
      false
    end
    alias :member? :include?

    def union(set)
      raise NotImplementedError
    end
    alias :+ :union

    def intersect(set)
      raise NotImplementedError
    end
    alias :** :intersect

    def subtract(set)
      raise NotImplementedError
    end
    alias :- :subtract

    def complement(set)
      raise NotImplementedError
    end

    # Returns +true+ if the receiver contains no element,
    # i.e., its count is 0.
    def empty?
      self.count == 0
    end

    private

    # Enlarge the size of buffer.
    def resize
      return self if self.count < (self.length * 0.811105)

      super(self.size * 2)
      @bitmap.resize
      self
    end
  end
end
