# encoding: utf-8

module OpenCL
  # NDArray (N-Dimension Array) is a Operand that 'shapes' its elements into
  # dimensions.
  #
  class NDArray < Operand

    def initialize(shape, type = :cl_float)
      shp = shape.is_a?(Array) ? shape : [shape]
      @shape = verify_shape(shp).clone.freeze
      super @shape.reduce(:*), type
    end

    attr_reader :shape

    undef :resize

    def dimension
      @shape.length
    end
    alias :dim :dimension

    def eclipse
    end

    def slice
    end

    def type_compatible?(nda)
      return false unless self.class == nda.class
      return (self.shape == nda.shape && self.type == nda.type)
    end

    private

    def verify_shape(shp)
      raise ArgumentError, "invalid shape, expected an Array." unless shp.is_a?(Array)
      shp.flatten.map do |d|
        raise ArgumentError, "invalid shape, expected an Array of numbers." unless d.respond_to?(:to_i)
        s = d.to_i
        raise ArgumentError, "invalid shape, dimension size should be positive." unless s > 0
        s
      end
    end
  end
end
