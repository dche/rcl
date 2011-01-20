# encoding: utf-8

module OpenCL

  class NDArray
    lib = Class.new(Library) do
      type :exact

      def_map_kernel(:rcl_abs_i, :abs) do |item|
        "#{item} >= 0 ? #{item} : -#{item}"
      end

      def_map_kernel(:rcl_sq_i, :square) do |item|
        "#{item} * #{item}"
      end
    end

    use lib
  end
end
