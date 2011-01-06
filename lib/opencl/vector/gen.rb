# encoding: utf-8

module OpenCL

  class Vector
    # do not bother to name the Library.
    lib = Class.new(Library) do
      type :number

      def_kernel(:rcl_vector_fill_value) do
        <<-EOT
__kernel void
rcl_vector_fill_value(__global T *vec, uint length, float number)
{
    int gid = get_global_id(0);
    if (gid < length) {
        vec[gid] = (T)(number);
    }
}
        EOT
      end

      def_method(:fill) do |number|
        execute_kernel :rcl_vector_fill_value, [self.length], :mem, self, :cl_uint, self.length, :cl_float, number
      end

      def_kernel(:rcl_vector_fill_range) do
        <<-EOT
__kernel void
rcl_vector_fill_range(__global T *vec, uint length, float start, float step)
{
    int gid = get_global_id(0);
    if (gid < length) {
        vec[gid] = (T)(mad((float)(gid), step, start));
    }
}
        EOT
      end

      def_method(:fill_range) do |start, step|
        execute_kernel :rcl_vector_fill_range, [self.length], :mem, self, :cl_uint, self.length, :cl_float, start, :cl_float, step
      end

      def_kernel(:rcl_vector_fill_rand) do
        <<-EOT
__kernel void
rcl_vector_fill_rand(__global T *vec, unsigned int length, uint seed)
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
        execute_kernel :rcl_vector_fill_rand, [self.length], :mem, self, :cl_uint, self.length, :cl_uint, seed
      end

    end

    use lib
  end

  class Vector

    class <<self
      # Creates a Vector with same length and type as of the parameter.
      def like(vector)
        self.new(vector.length, vector.type.tag)
      end

      # Creates a Vector and fill it with all ones.
      def ones(length, type = :cl_float)
        raise ArgumentError, "can't initialize a Vector of structures to all ones." unless Type.new(type).number?
        self.new(length, type).fill(1)
      end

      def ones_like(vector)
        vec = self.like(vector)
        raise ArgumentError, "can't initialize a Vector of structures to all ones." unless vector.type.number?
        vec.fill(1)
      end

      def range(range, step = 1, type = :cl_float)
        t = Type.new(type)
        raise ArgumentError, "expected a scalar data type" unless t.scalar?

        start = range.first
        stop = range.last

        raise ArgumentError, "expected an interval" if start == stop
        if t.integer?
          [start, stop, step].each do |v|; v.round; end
        end
        raise ArgumentError, "expected the step is not 0" if step == 0

        step = -step if (stop - start) * step < 0
        asc = step > 0

        len = ((stop - start) / step + 1).ceil
        if len == 1
          len = 2
          step = stop - start
        end
        self.new(len, type).fill_range(start, step)
      end

      def rand(length, type = :cl_float)
        self.new(length, type).rand
      end
    end

  end
end
