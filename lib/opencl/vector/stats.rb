# encoding: utf-8

module OpenCL
  class Vector
    lib = Class.new(Library) do
      type :float

      # Returns the arithmetic mean of the receiver.
      def_method(:mean) do
        self.summary / self.length
      end
      alias_method :arithmetic_mean, :mean

      # Returns the weighted mean of the receiver.
      #
      # weights:: a Vector contains the weights. It must has same lenght as of
      #           the receiver.
      def_method(:average) do |weights|
        (self.clone * weights).summary / self.length
      end
      alias_method :weighted_mean, :average

      # Returns the root mean square (RMS) of the receiver.
      def_method(:rms) do
        self.clone.square.mean.sqrt
      end
      alias_method :root_mean_square, :rms

      # Returns the standard deviation of the receiver.
      def_method(:std) do
        (self.clone - self.mean).rms
      end
      alias_method :standard_deviation, :std

    end

    use lib
  end
end
