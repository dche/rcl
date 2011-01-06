# encoding: utf-8

module OpenCL
  # A Vector object is a Operand (hence a OpenCL Buffer) stores a fixed number
  # of elements. Each element can be referenced by its position.
  #
  # Please don't confuse this class with the built-in OpenCL
  # vector types (e.g., :cl_float 16)
  class Vector < Operand

    def initialize(length, type = :cl_float)
      super length, type
    end

    def type_compatible?(vector)
      return false unless vector.is_a?(self.class)
      return (self.length == vector.length && self.type == vector.type)
    end

    def to_s
      s = '['
      s << [self.length, 32].min.times.map do |i|; self[i]; end.join(', ')
      s << ", ..." if self.length > 32
      s << ']'
    end
  end
end
