# encoding: utf-8

module OpenCL
  # A Vector object is one dimension NDArray.
  #
  # Please don't confuse this class with the built-in OpenCL
  # vector types (e.g., :cl_float 16)
  class Vector < NDArray

    def to_s
      s = '['
      s << [self.length, 32].min.times.map do |i|; self[i]; end.join(', ')
      s << ", ..." if self.length > 32
      s << ']'
    end
  end
end
