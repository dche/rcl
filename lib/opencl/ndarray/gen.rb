# encoding: utf-8

module OpenCL

  class NDArray
    # do not bother to name the Library.
    lib = Class.new(Library) do
      type :number

      def_kernel(:rcl_ndarray_fill_value) do
        <<-EOT
__kernel void
rcl_ndarray_fill_value(__global T *vec, uint length, float number)
{
    int gid = get_global_id(0);
    if (gid < length) {
        vec[gid] = (T)(number);
    }
}
        EOT
      end

      def_method(:fill) do |number|
        execute_kernel :rcl_ndarray_fill_value, [self.length], :mem, self, :cl_uint, self.length, :cl_float, number
      end

      def_kernel(:rcl_ndarray_fill_rand) do
        <<-EOT
__kernel void
rcl_ndarray_fill_rand(__global T *vec, unsigned int length, uint seed)
{
    int gid = get_global_id(0);
    if (gid < length) {
        rcl_random_state rst;
        rcl_srandom(&rst, seed);

        vec[gid] = (T)(rcl_randomf(&rst));
    }
}
        EOT
      end

      def_method(:rand) do |seed = nil|
        seed = Kernel.rand(Time.now.to_i) if seed.nil?
        execute_kernel :rcl_ndarray_fill_rand, [self.length], :mem, self, :cl_uint, self.length, :cl_uint, seed
      end

    end

    use lib
  end

  class NDArray

    class <<self
      # Creates a NDArray with same length and type as of the parameter.
      def like(na)
        self.new(na.shape, na.type.tag)
      end

      # Creates a NDArray and fill it with all ones.
      def ones(shape, type = :cl_float)
        raise ArgumentError, "can't initialize a NDArray of structures to all ones." unless Type.new(type).number?
        self.new(shape, type).fill(1)
      end

      def ones_like(na)
        nary = self.like(na)
        raise ArgumentError, "can't initialize a NDArray of structures to all ones." unless na.type.number?
        nary.fill(1)
      end

      def rand(shape, type = :cl_float)
        self.new(shape, type).rand
      end
    end

  end
end
