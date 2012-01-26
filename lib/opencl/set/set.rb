# encoding: utf-8

module OpenCL
  # Set is a pool of comparable elements.
  class Set < Operand
    # Creates a Set.
    def initialize(type, options = {})
      cap = options[:capacity] || 1023
      alloc = options[:allocation] || :random

      sz = next_pow2(cap.to_i + 1)
      sz = 1024 if sz < 1024

      super sz, type
      @bitmap = Bitmap.new sz, alloc
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
    #
    # Note: The index +0+ is never used.
    def put(elm)
      resize

      i = self.length
      i = @bitmap.next_cell + 1 while i == self.length

      begin
        self[i] = elm
        @bitmap.mark_cell(i - 1)
      rescue Exception => e
        # TODO: what about ignoring the operation?
        raise e
      end
      i
    end

    def [](idx)
      return nil unless @bitmap.set?(idx - 1)
      super
    end

    protected :[]=

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
      # garbage collecting
      self.gc if self.respond_to? :gc

      sz = self.size * 2
      super(sz)
      @bitmap.resize(sz)
      self
    end
  end
end
