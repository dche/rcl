# encoding: utf-8

module OpenCL
  class Library
    def self.def_reduction_kernel(kernel_name, method_name, &blk)
      tmpl = <<-EOT
__kernel void
<%= kernel_name %>(const __global T *vec,
                   __global T *out,
                   __local T *shared,
                   int reduction_size,
                   int n
                  )
{

#define BIN_OP(x, y)  (<%= bin_op %>)

    // load data of current work group from global to shared memroy
    int lid = get_local_id(0);
    int gid = get_group_id(0);
    int group_size = get_local_size(0);
    int group_number = get_num_groups(0);

    int i = gid * group_size + lid;
    shared[lid] = vec[i];

    int iter = 1;
    while (iter < reduction_size) {
        i += (group_size * group_number);
        if (i < n) {
          shared[lid] = BIN_OP(shared[lid], vec[i]);
        }
        iter += 1;
    }

    // each work item has reduced its portion, we got #<group_size> values
    // in shared memory. reduce them to a single value.

    barrier(CLK_LOCAL_MEM_FENCE);
    if (group_size >= 512) {
        if (lid < 256) {
            shared[lid] = BIN_OP(shared[lid], shared[lid + 256]);
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    if (group_size >= 256) {
        if (lid < 128) {
            shared[lid] = BIN_OP(shared[lid], shared[lid + 128]);
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    if (group_size >= 128) {
        if (lid < 64) {
            shared[lid] = BIN_OP(shared[lid], shared[lid + 64]);
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    if (group_size >= 64) {
        if (lid < 32) {
            shared[lid] = BIN_OP(shared[lid], shared[lid + 32]);
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    if (group_size >= 32) {
        if (lid < 16) {
            shared[lid] = BIN_OP(shared[lid], shared[lid + 16]);
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    if (group_size >= 16) {
        if (lid < 8) {
            shared[lid] = BIN_OP(shared[lid], shared[lid + 8]);
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    if (group_size >= 8) {
        if (lid < 4) {
            shared[lid] = BIN_OP(shared[lid], shared[lid + 4]);
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    if (group_size >= 4) {
        if (lid < 2) {
            shared[lid] = BIN_OP(shared[lid], shared[lid + 2]);
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
    if (group_size >= 2) {
        if (lid < 1) {
            shared[0] = BIN_OP(shared[0], shared[1]);
        }
    }

#undef BIN_OP

    barrier(CLK_LOCAL_MEM_FENCE);
    if (lid == 0) {
        out[gid] = shared[0];
    }
}
      EOT

      bin_op = yield('x', 'y').chomp.gsub("\n", "\\")
      self.def_kernel(kernel_name) do
        ERB.new(tmpl).result(binding)
      end

      def_method(method_name) do
        self.reduce(kernel_name)
      end
    end
  end
end
