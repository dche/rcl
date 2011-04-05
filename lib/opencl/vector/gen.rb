# encoding: utf-8

module OpenCL

  class Vector
    # do not bother to name the Library.
    lib = Class.new(Library) do
      type :number

      def_kernel(:rcl_vector_fill_range) do
        <<-EOT
__kernel void
rcl_vector_fill_range(__global T *vec, int length, float start, float step)
{
    int gid = get_global_id(0);
    if (gid < length) {
        vec[gid] = (T)(mad((float)(gid), step, start));
    }
}
__kernel void
rcl_vector_fill_range_log(__global T *vec, int length, float start, float step, float base)
{
    int gid = get_global_id(0);
    if (gid < length) {
        float pw = mad((float)(gid), step, start);
        vec[gid] = (T)pow(base, pw);  // CHECK: native_pow is not always available.
    }
}
        EOT
      end

      def_method(:fill_range) do |start, step|
        execute_kernel :rcl_vector_fill_range, [self.length], :mem, self, :cl_int, self.length, :cl_float, start, :cl_float, step
      end

      def_method(:fill_range_log) do |start, step, base|
        raise ArgumentError, "logarithm base should be larger than 0." if base <= 0
        execute_kernel :rcl_vector_fill_range_log,
                       [self.length],
                       :mem, self, :cl_int, self.length,
                       :cl_float, start, :cl_float, step, :cl_float, base
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
