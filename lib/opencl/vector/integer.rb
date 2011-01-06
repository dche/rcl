# encoding: utf-8

module OpenCL

  class Vector
    lib = Class.new(Library) do
      type :exact

      def_map_kernel(:rcl_abs_v_i, :abs) do |item|
        "#{item} >= 0 ? #{item} : -#{item}"
      end

      def_map_kernel(:rcl_sq_v_i, :square) do |item|
        "#{item} * #{item}"
      end
    end

    use lib
  end
end
