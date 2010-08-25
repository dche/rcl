
require 'erb'

module OpenCL
  module DSP
    module FFTKernelTemplates
      private
      
      def next_kernel_name
        "fft#{@kernels.length}"
      end
      
      def kernel(name, max_radix, body)
        args = if self.data_format == :split_complex
          '__global float *in_real, __global float *in_imag, __global float *out_real, __global float *out_imag, int dir, int S'
        else
          '__global float2 *in, __global float2 *out, int dir, int S'
        end
        
        tp = <<-EOT
__kernel void <%= name %> (<%= args %>)
{
    int i, j, r, indexIn, indexOut, index, tid, bNum, xNum, k, l;
    int s, ii, jj, offset;
    float2 w;
    float ang, angf, ang1;
    __local float *lMemStore, *lMemLoad;
    float2 a[<%= max_radix %>];
    int lId = get_local_id(0);
    int groupId = get_group_id(0);
    
    <%= body %>
}
        EOT
        ERB.new(tp).result(binding)
      end
      
      def formatted_load(aIndex, gIndex)
        if self.data_format == :interleaved_complex
          "a[#{aIndex}] = in[#{gIndex}];\n"
        else
          "a[#{aIndex}].x = in_real[#{gIndex}];\n"
          "a[#{aIndex}].y = in_image[#{gIndex}];\n"
        end
      end
      
      def formatted_store(aIndex, gIndex)
        if self.data_format == :interleaved_complex
          "out[#{gIndex}] = a[#{aIndex}];\n"
        else
          "out_real[#{gIndex}] = a[#{aIndex}].x;\n"
          "out_image[#{gIndex}] = a[#{aIndex}].y;\n"
        end
      end
      
      def fft_kernel(size, pass)
        tp = <<-EOT
        <% pass.times do |i| %>
    fftKernel<%= size %>(a + <%= i * size %>, dir);
        <% end %>
        EOT
        ERB.new(tp).result(binding)
      end
      
      def twiddle_kernel(body, nr, numIter, n_prev, len, numWorkItemsPerXForm)
        log_n_prev = Math.log2(n_prev).to_i
        
        tp = <<-EOT
          <% numIter.times do |z| %>
            <% if z == 0 %>
              <% if n_prev > 1 %>
    angf = (float)(ii >> <%= log_n_prev %>);
              <% else %>
    angf = (float) ii;
              <% end %>
            <% else %>
              <% if n_prev > 1 %>
    angf = (float)(<%= z * numWorkItemsPerXForm %> + ii) >> <%= log_n_prev %>);
              <% else %>
    angf = (float)(<%= z * numWorkItemsPerXForm %>);
              <% end %>
            <% end %>
            <% nr.times do |k| %>
              <% ind = z * nr + k %>
    ang = dir * (2.0f * M_PI * <%= k %>.0f / <%= len %>.0f) * angf;
    w = (float2)(native_cos(ang), native_sin(ang));
    a[<%= ind %>] = complexMul(a[<%= ind %>], w);
            <% end %>
          <% end %>
        EOT
        body << ERB.new(tp).result(binding)
      end
      
      def local_load_index_arith(body, n_prev, nr, numWorkItemsReq, numWorkItemsPerXForm, numXFormsPerWG, offset, mid_pad)
        n_curr = n_prev * nr
        log_n_curr = Math.log2(n_curr).to_i
        log_n_prev = Math.log2(n_prev).to_i
        incr = (numWorkItemsReq + offset) * nr + mid_pad
        
        if n_curr < numWorkItemsPerXForm
          body << if n_prev == 1
            "    j = ii & #{n_curr - 1};\n"
          else
            "    j = (ii & #{n_curr - 1}) >> #{log_n_prev};\n"
          end
          
          body << if n_prev == 1
            "    i = ii >> #{log_n_curr};\n"
          else
            "    i = mad24(ii >> #{log_n_curr}, #{n_prev}, ii & #{n_prev - 1});\n"
          end
        else
          body << if n_prev == 1
            "    j = ii;\n"
          else
            "    j = ii >> #{log_n_prev};\n"
          end
          
          body << if n_prev == 1
            "    i = 0;\n"
          else
            "    i = ii & #{n_prev - 1};\n"
          end
        end
        
        if numXFormsPerWG > 1
          body << "    i = mad24(jj, #{incr}, i);\n"
        end
        
        body << "    lMemLoad = sMem + mad24(j, #{numWorkItemsReq + offset}, i);\n"
      end
      
      def local_store_index_arith(body, numWorkItemsReq, numXFormsPerWG, nr, offset, mid_pad)
        body << if numXFormsPerWG == 1
          "    lMemStore = sMem + ii;\n"
        else
          "    lMemStore = sMem + mad24(jj, #{(numWorkItemsReq + offset) * nr + mid_pad}, ii);\n"
        end
      end
      
      def get_padding(numWorkItemsPerXForm, n_prev, numWorkItemsReq, numXFormsPerWG, nr, numBanks)
        lmem_size = offset = mid_pad = 0
        
        if numWorkItemsPerXForm > n_prev && n_prev < numBanks
          numRowsReq = ((numWorkItemsPerXForm < numBanks) ? numWorkItemsPerXForm : numBanks) / n_prev
          numColsReq = 1
          if numRowsReq > nr
            numColsReq = numRowsReq / nr
          end
          offset = n_prev * numColsReq
        end
        
        if numWorkItemsPerXForm < numBanks && numXFormsPerWG != 1
          bankNum = ((numWorkItemsReq + offset) * nr) & (numBanks - 1)
          if bankNum < numWorkItemsPerXForm
            mid_pad = numWorkItemsPerXForm - bankNum
          end
        end
        
        lmem_size = (numWorkItemsReq + offset) * nr * numXFormsPerWG + mid_pad * (numXFormsPerWG - 1)
        
        return [lmem_size, offset, mid_pad]
      end
      
      def local_loads(body, n, nr, nr1, n_prev, n_curr, numWorkItemsPerXForm, numWorkItemsReq, offset, comp)
        numWorkItemsReqN = n / nr
        interBlockHNum = [n_prev / numWorkItemsPerXForm, 1].max
        interBlockHStride = numWorkItemsPerXForm
        vertWidth = [numWorkItemsPerXForm, 1].max
        vertWidth = [vertWidth, nr].min
        vertNum = nr / vertWidth
        vertStride = (n / nr + offset) * vertWidth
        iter = [numWorkItemsReqN / numWorkItemsPerXForm, 1].max
        intraBlockHStride = [numWorkItemsPerXForm / (n_prev * nr), 1].max
        intraBlockHStride *= n_prev
        
        stride = numWorkItemsReq / nr1
        
        iter.times do |i|
          ii = i / (interBlockHNum * vertNum)
          zz = i % (interBlockHNum * vertNum)
          jj = zz % interBlockHNum
          kk = zz / interBlockHNum
          nr1.times do |z|
            st = kk * vertStride + jj * interBlockHStride + ii * intraBlockHStride + z * stride
            body << "    a[#{i *  nr1 + z}].#{comp} = lMemLoad[#{st}];\n"
          end
        end
        body << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
      end
      
      def local_stores(body, numIter, nr, numWorkItemsPerXForm, numWorkItemsReq, offset, comp)
        numIter.times do |z|
          nr.times do |k|
            index = k * (numWorkItemsReq + offset) + z * numWorkItemsPerXForm
            body << "    lMemStore[#{index}] = a[#{z * nr + k}].#{comp};\n"
          end
        end
        body << "    barrier(CLK_LOCAL_MEM_FENCE);\n"
      end
      
      def global_loads_and_transpose(body, n, numWorkItemsPerXForm, numXFormsPerWG, r0)
        lgNumWorkItemsPerXForm = Math.log2(numWorkItemsPerXForm).to_i
        groupSize = numWorkItemsPerXForm * numXFormsPerWG
        lmem_size = 0
        
        if numXFormsPerWG > 1
          body << "    s = S * #{numXFormsPerWG - 1};\n"
        end
        if numWorkItemsPerXForm >= @mem_coalesce_width
          if numXFormsPerWG > 1
            tp = <<-EOT
    ii = lId & <%= numWorkItemsPerXForm - 1 %>;
    jj = lId >> <%= lgNumWorkItemsPerXForm %>;
    if (!s || (groupID < get_num_groups(0) - 1) || (jj < s)) {
        offset = mad24(mad24(groupId, <%= numXFormsPerWG %>, jj), <%= n %>, ii);
                  <% if self.data_format == :interleaved_complex %>
        in += offset;
        out += offset;
                  <% else %>
        in_real += offset;
        in_image += offset;
        out_real += offset;
        out_image += offset;
                  <% end %>
                  <% r0.times do |i| %>
        <%= formatted_load i, i * numWorkItemsPerXForm %>
                  <% end %>
    }
            EOT
            body << ERB.new(tp).result(binding)
          else
            tp = <<-EOT
    ii = lId;
    jj = 0;
    offset = mad24(groupId, <%= n %>, ii);              
              <% if self.data_format == :interleaved_complex %>
    in += offset;
    out += offset;
              <% else %>
    in_real += offset;
    in_image += offset;
    out_real += offset;
    out_image += offset;
              <% end %>
              <% r0.times do |i| %>
    <%= formatted_load i, i * numWorkItemsPerXForm %>
              <% end %>
            EOT
            body << ERB.new(tp).result(binding)
          end
        elsif n > @mem_coalesce_width
          numInnerIter = n / @mem_coalesce_width
          numOuterIter = numXFormsPerWG / (groupSize / @mem_coalesce_width)

          tp =<<-EOT
    ii = lId & <%= @mem_coalesce_width - 1 %>;
    jj = lId >> <%= Math.log2(@mem_coalesce_width).to_i %>;
    lMemStore = sMem + mad24(jj, <%= n + numWorkItemsPerXForm %>, ii);
    offset = mad24(groupId, <%= numXFormsPerWG %>, jj);
    offset = mad24(offset, <%= n %>, ii);

            <% if self.data_format == :interleaved_complex %>
    in += offset;
    out += offset;
            <% else %>
    in_real += offset;
    in_image += offset;
    out_real += offset;
    out_image += offset;
            <% end %>

    if ((groupId == get_num_groups(0) - 1) && s) {
            <% numOuterIter.times do |i| %>
        if (j < s) {
              <% numInnerIter.times do |j| %>
            <%= formatted_load i * numInnerIter + j, j * @mem_coalesce_width + i * (groupSize / @mem_coalesce_width) * n %>
              <% end %>
        }
              <% if i != numOuterIter - 1 %>
        jj += <%= groupSize / @mem_coalesce_width %>;
              <% end %>
            <% end %>
    } else {
            <% numOuterIter.times do |i| %>
              <% numInnerIter.times do |j| %>
        <%= formatted_load i * numInnerIter + j, j * @mem_coalesce_width + i * (groupSize / @mem_coalesce_width) * n %>
              <% end %>
            <% end %>
    }
    ii = lId & <%= numWorkItemsPerXForm - 1 %>;
    jj = lId >> <%= lgNumWorkItemsPerXForm %>;
    lMemLoad = sMem + mad24(jj, <%= n + numWorkItemsPerXForm %>, ii);

            <% numOuterIter.times do |i| %>
              <% numInnerIter.times do |j| %>
    lMemStore[<%= j * @mem_coalesce_width + i * (groupSize / @mem_coalesce_width) * (n + numWorkItemsPerXForm) %>] = a[<%= i * numInnerIter + j %>].x;            
              <% end %>
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );

            <% r0.times do |i| %>
    a[<%= i %>].x = lMemStore[<%= i * numWorkItemsPerXForm %>];
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );

            <% numOuterIter.times do |i| %>
              <% numInnerIter.times do |j| %>
    lMemStore[<%= j * @mem_coalesce_width + i * (groupSize / @mem_coalesce_width) * (n + numWorkItemsPerXForm) %>] = a[<%= i * numInnerIter + j %>].y;            
              <% end %>
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );

            <% r0.times do |i| %>
    a[<%= i %>].y = lMemStore[<%= i * numWorkItemsPerXForm %>];
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );
          EOT
          body << ERB.new(tp).result(binding)
          lmem_size = (n + numWorkItemsPerXForm) * numXFormsPerWG
        else
          tp =<<-EOT
    offset = mad24(groupId, <%= n * numXFormsPerWG %>, lId);
    
            <% if self.data_format == :interleaved_complex %>
    in += offset;
    out += offset;
            <% else %>
    in_real += offset;
    in_image += offset;
    out_real += offset;
    out_image += offset;
            <% end %>
    ii = lId & <%= n - 1 %>;
    jj = lId >> <%= Math.log2(n).to_i %>;
    lMemStore = sMem + mad24(jj, <%= n + numWorkItemsPerXForm %>, ii);
    
    if ((groupId == get_num_groups(0) - 1) && s) {
            <% r0.times do |i| %>
        if (jj < s)
            <%= formatted_load i, i * groupSize %>
              <% if i != r0 - 1 %>
            jj += <%= groupSize / n %>;
              <% end %>
            <% end %>
    } else {
            <% r0.times do |i| %>
        <%= formatted_load i, i * groupSize %>
            <% end %>             
    }
            <% if numWorkItemsPerXForm > 1 %>
    ii = lId & <%= numWorkItemsPerXForm - 1 %>;
    jj = lId >> <%= lgNumWorkItemsPerXForm %>;
    lMemLoad = sMem + mad24(jj, <%= n + numWorkItemsPerXForm %>, ii);
            <% else %>
    ii = 0;
    jj = lId;
    lMemLoad = sMem + mul24(jj, <%= n + numWorkItemsPerXForm %>);
            <% end %>    
            <% r0.times do |i| %>
    lMemStore[<%= i * (groupSize / n) * (n + numWorkItemsPerXForm) %>] = a[<%= i %>].x;
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );          
            <% r0.times do |i| %>
    a[<%= i %>].x = lMemLoad[<%= i * numWorkItemsPerXForm %>];
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );         
            <% r0.times do |i| %>
    lMemStore[<%= i * (groupSize / n) * (n + numWorkItemsPerXForm) %>] = a[<%= i %>].y;
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );            
            <% r0.times do |i| %>
    a[<%= i %>].y = lMemLoad[<%= i * numWorkItemsPerXForm %>];
            <% end %>             
    barrier( CLK_LOCAL_MEM_FENCE );
          EOT
          body << ERB.new(tp).result(binding)
          lmem_size = (n + numWorkItemsPerXForm) * numXFormsPerWG
        end
        lmem_size
      end
      
      def global_stores_and_transpose(body, n, max_radix, nr, numWorkItemsPerXForm, numXFormsPerWG)
        groupSize = numWorkItemsPerXForm * numXFormsPerWG
        lmem_size = 0
        numIter = max_radix / nr
        
        if numWorkItemsPerXForm >= @mem_coalesce_width
          tp = <<-EOT
            <% if numXFormsPerWG > 1 %>
    if (!s || (groupId < get_num_groups(0) - 1)) {
            <% end %>
            <% max_radix.times do |i| %>
              <% j = i % numIter; k = i / numIter; ind = j * nr + k %>
    <%= formatted_store ind, i * numWorkItemsPerXForm %>
            <% end %>
            <% if numXFormsPerWG > 1 %>
    }
            <% end %>
          EOT
          body << ERB.new(tp).result(binding)
        elsif n >= @mem_coalesce_width
          numInnerIter = n / @mem_coalesce_width
          numOuterIter = numXFormsPerWG / (groupSize / @mem_coalesce_width)
          
          tp = <<-EOT
    lMemLoad = sMem + mad24(jj, <%= n + numWorkItemsPerXForm %>, ii);
    ii = lId & <%= @mem_coalesce_width - 1 %>;
    jj = lId >> <%= Math.log2(@mem_coalesce_width).to_i %>;
    lMemStore = sMem + mad24(jj, <%= n + numWorkItemsPerXForm %>, ii);
            <% max_radix.times do |i| %>
              <% j = i % numIter; k = i / numIter; ind = j * nr + k %>
    lMemLoad[<%= i * numWorkItemsPerXForm %>] = a[<%= ind %>].x;
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );
            <% numOuterIter.times do |i| %>
              <% numInnerIter.times do |j| %>
    a[<%= i * numInnerIter + j %>].x = lMemStore[<%= j * @mem_coalesce_width + i * (groupSize / @mem_coalesce_width) * (n + numWorkItemsPerXForm) %>];
              <% end %>
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );
            <% max_radix.times do |i| %>
              <% j = i % numIter; k = i / numIter; ind = j * nr + k %>
    lMemLoad[<%= i * numWorkItemsPerXForm %>] = a[<%= ind %>].y;
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );
            <% numOuterIter.times do |i| %>
              <% numInnerIter.times do |j| %>
    a[<%= i * numInnerIter + j %>].y = lMemStore[<%= j * @mem_coalesce_width + i * (groupSize / @mem_coalesce_width) * (n + numWorkItemsPerXForm) %>];
              <% end %>
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );
    if ((groupId == get_num_groups(0) - 1) && s) {
            <% numOuterIter.times do |i| %>
    if (jj < s) {
              <% numInnerIter.times do |j| %>
        <%= formatted_store i * numInnerIter + j, j * @mem_coalesce_width + i * (groupSize / @mem_coalesce_width) * n %>
              <% end %>
    }
              <% if i != numOuterIter - 1 %> 
    jj += <%= groupSize / @mem_coalesce_width %>
              <% end %>
            <% end %>
    } else {
          <% numOuterIter.times do |i| %>
            <% numInnerIter.times do |j| %>
        <%= formatted_store i * numInnerIter + j, j * @mem_coalesce_width + i * (groupSize / @mem_coalesce_width) * n %>
            <% end %>
          <% end %>
    }
          EOT
          body << ERB.new(tp).result(binding)
          lmem_size = (n + numWorkItemsPerXForm) * numXFormsPerWG
        else
          tp = <<-EOT
    lMemLoad = sMem + mad24(jj, <%= n + numWorkItemsPerXForm %>, ii);
    ii =lId & <%= n - 1 %>;
    jj = lId >> <%= Math.log2(n).to_i %>;
    lMemStore = sMem + mad24(jj, <%= n + numWorkItemsPerXForm %>, ii);    
            <% max_radix.times do |i| %>
              <%= j = i % numIter; k = i / numIter; ind = j * nr + k %>
    lMemLoad[<%= i * numWorkItemsPerXForm %>] = a[<%= ind %>].x;
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );
            <% max_radix.times do |i| %>
    a[<%= i %>].x = lMemStore[<%= i * (groupSize / n) * (n + numWorkItemsPerXForm) %>];
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );
            <% max_radix.times do |i| %>
              <%= j = i % numIter; k = i / numIter; ind = j * nr + k %>
    lMemLoad[<%= i * numWorkItemsPerXForm %>] = a[<%= ind %>].y;
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );
            <% max_radix.times do |i| %>
    a[<%= i %>].y = lMemStore[<%= i * (groupSize / n) * (n + numWorkItemsPerXForm) %>];
            <% end %>
    barrier( CLK_LOCAL_MEM_FENCE );
    if ((groupId == get_num_groups(0) - 1) && s) {
            <% max_radix.times do |i| %>
        if (jj < s) {
            <% formatted_store i, i * groupSize %>
        }
              <% if i != max_radix - 1 %>
        jj += <%= groupSize / n %>;
              <% end %>
            <% end %>
    } else {
            <% max_radix.times do |i| %>
        <%= formatted_store i, i * groupSize %>
            <% end %>
    }
          EOT
          body << ERB.new(tp).result(binding)
          lmem_size = (n + numWorkItemsPerXForm) * numXFormsPerWG
        end
        lmem_size
      end
      
      #
      def local_fft_kernel_string(n, radix_array)
        kernel = {}
        kernel[:name] = next_kernel_name
        kernel[:axis] = :x
        kernel[:inplace_possible] = true
        
        max_radix = radix_array.first
        body = ''
        
        numWorkItemsPerXForm = n / max_radix
        numWorkItemsPerWG = numWorkItemsPerXForm <= 64 ? 64 : numWorkItemsPerXForm
        numXFormsPerWG = numWorkItemsPerWG / numWorkItemsPerXForm;
        kernel[:num_workgroup] = numXFormsPerWG
        kernel[:num_workitem_per_workgroup] = numWorkItemsPerWG
        kernel[:lmem_size] = global_loads_and_transpose(body, n, 
                                                        numWorkItemsPerXForm,
                                                        numXFormsPerWG,
                                                        max_radix)
        
        # TODO: sanity checks.
        n_prev = 1
        len = n
        radix_array.length.times do |r|
          nr = radix_array[r]
          numIter = max_radix / nr
          numWorkItemsReq = n / nr
          n_curr = n_prev * nr;
          fft_kernel(nr, numIter)

          if r < radix_array.length - 1
            nr1 = radix_array[r + 1]

            twiddle_kernel(body, nr, numIter, n_prev, len, numWorkItemsPerXForm)
            lmem_size, offset, mid_pad = get_padding(numWorkItemsPerXForm, n_prev, numWorkItemsReq, numXFormsPerWG, nr, @local_mem_banks_number)
            kernel[:lmem_size] = [kernel[:lmem_size], lmem_size].max
            local_store_index_arith(body, numWorkItemsReq, numXFormsPerWG, nr, offset, mid_pad)
            local_load_index_arith(body, n_prev, nr, numWorkItemsReq, numWorkItemsPerXForm, numXFormsPerWG, offset, mid_pad)
            local_stores(body, numIter, nr, numWorkItemsPerXForm, numWorkItemsReq, offset, :x)
            local_loads(body, n, nr, nr1, n_prev, n_curr, numWorkItemsPerXForm, numWorkItemsReq, offset, :x)
            local_stores(body, numIter, nr, numWorkItemsPerXForm, numWorkItemsReq, offset, :y)
            local_loads(body, n, nr, nr1, n_prev, n_curr, numWorkItemsPerXForm, numWorkItemsReq, offset, :y)

            n_prev = n_curr
            len = len / radix_array[r]
          end
        end

        lmem_size = global_stores_and_transpose(body, n, max_radix, 
                                               radix_array[-1],
                                               numWorkItemsPerXForm,
                                               numXFormsPerWG)
        kernel[:lmem_size] = [kernel[:lmem_size], lmem_size].max
        if kernel[:lmem_size] > 0
          body = "    __local float sMem[#{kernel[:lmem_size]}];\n" + body
        end
        kernel[:source] = kernel(kernel[:name], max_radix, body)
        @kernels << kernel
      end
      
      #
      def global_radix_info(n)
        base_radix = [n, 128].min
        en = n;
        num_r = 0;
        while en > base_radix
          en /= base_radix
          num_r += 1;
        end
        
        radix = (1..num_r).to_a.map do; base_radix; end
        radix[num_r] = en
        
        r1 = []
        r2 = []
        (num_r + 1).times do |i|
          b = radix[i]
          if b <= 8
            r1[i] = b;
            r2[i] = 1;
            next
          end
          
          v1 = 2
          v2 = b / v1
          while v2 > v1
            v1 *= 2
            v2 = b / v1
          end
          r1[i] = v1
          r2[i] = v2
        end
        [radix, r1, r2]
      end
      
      #
      def global_fft_kernel_string(n, bs, axis, vert_bs)
        max_threads_per_block = @max_workitem_size
        max_array_len = @max_radix
        batch_size = @mem_coalesce_width
        vertical = (axis != :x)
        
        radixArray, r1Array, r2Array = global_radix_info(n)
        
        m = Math.log2(n).to_i
        r_init = vertical ? bs : 1
        batch_size = [bs, batch_size].min if vertical
        
        pass_number = radixArray.length      
        pass_number.times do |i|
          radix = radixArray[i]
          r1 = r1Array[i]
          r2 = r2Array[i]
          
          strideI = r_init * radixArray.reduce(:*) / radixArray[i]
          strideO = r_init * (radixArray.take(i + 1).reduce(:*))
          
          threadsPerXForm = r2
          batch_size = @max_workitem_size if r2 == 1
          batch_size = [batch_size, strideI].min
          threadsPerBlock = [max_threads_per_block, batch_size * threadsPerXForm].min
          batch_size = threadsPerBlock / threadsPerXForm
          
          numIter = r1 / r2
          gInInc = threadsPerBlock / batch_size
          
          lgStrideO = Math.log2(strideO)
          numBlocksPerXForm = strideI / batch_size
          numBlocks = numBlocksPerXForm
          numBlocks *= vertical ? bs : vert_bs
          
          kernel = { :name => next_kernel_name }
          if r2 == 1
            kernel[:lmem_size] = 0;
          else
            kernel[:lmem_size] = strideO == 1 ? (radix + 1) * batch_size : threadsPerBlock * r1
          end
          kernel[:num_workgroup] = numBlocks
          kernel[:num_workitem_per_workgroup] = threadsPerBlock
          kernel[:axis] = axis
          kernel[:inplace_possible] = (i == pass_number - 1 && pass_number.odd?)
          
          tp = <<-EOT
            <% if kernel[:lmem_size] > 0 %>
    __local float sMem[<%= kernel[:lmem_size] %>];
            <% end %>
            <% if vertical %>
    xNum = groupId >> <%= Math.log2(numBlocksPerXForm).to_i %>;
    groupId = groupId & <%= numBlocksPerXForm - 1 %>;
    indexIn = mad24(groupId, <%= batch_size %>, xNum << <%= Math.log2(n * bs).to_i %>);
    tid = mul24(groupId, <%= batch_size %>);
    i = tid >> <%= lgStrideO %>;
    j = tid && <%= strideO - 1 %>;
              <% stride = radix * r_init * radixArray.take(i + 1).reduce(:*) %>
    indexOut = mad24(i, <%= stride %>, j + (xNum << <%= Math.log2(n * bs).to_i %>));
    bNum = groupId;
            <% else %>
              <% lgNumBlocksPerXForm = Math.log2(numBlocksPerXForm).to_i %>
    bNum = groupId & <%= numBlocksPerXForm - 1 %>
    xNum = groupId >> <%= Math.log2(numBlocksPerXForm).to_i %>;
    indexIn = mul24(bNum, <%= batch_size %>);
    tid = IndexIn;
    i = tid >> <%= lgStrideO %>;
    j = tid && <%= strideO - 1 %>;
              <% stride = radix * r_init * radixArray.take(i + 1).reduce(:*) %>
    indexOut = mad24(i, <%= stride %>, j);
    indexIn += xNum << <%=  m %>;
    indexOut += xNum << <%= m %>;
            <% end %>
    // load data 
            <% lgBatchSize = Math.log2(batch_size).to_i %>
    tId = lId;
    i = tId & <%= batch_size - 1 %>;
    j = tId >> <%= lgBatchSize %>;
    indexIn += mad24(j, <%= strideI %>, i);
            <% if self.data_format == :split_complex %>
    in_real += indexIn;
    in_image += indexIn;  
              <% r1.times do |j| %>
    a[<%= j %>].x = in_real[<%%>];
              <% end %>
              <% r1.times do |j| %>
    a[<%= j %>].y = in_image[<%%>];
              <% end %>
            <% else %>
    in += IndexIn;
              <% r1.times do |j| %>
    a[<%= j %>] = in[<%%>];
              <% end %>
            <% end %>  
    fftKernel<%= r1 %>(a, dir);  
            <% if r2 > 1 %>
    // twiddle
              <% 1.upto(r1) do |k| %>
    ang = dir * (2.0f * M_PI * <%= k %> / <%= radix %>) * j;
    w = (float2)(native_cos(ang), native_sin(ang));
    a[<%= k %>] = complexMul(a[<%= k %>, w]);
              <% end %>
    // shuffle
    indexIn = mad24(j, <%= threadsPerBlock * numIter %>, i);
    lMemStore = sMem + tid;
    lMemLoad = sMem + indexIn;
              <% r1.times do |k| %>
    lMemStore[<%= k * threadsPerBlock %>] = a[<%= k %>].x;
              <% end %>
    barrier(CLK_LOCAL_MEM_FENCE);
              <% numIter.times do |k| %>
                <% r2.times do |t| %>
    a[<%= k * r2 + t %>].x = lMemLoad[<%= t * batch_size + k * threadsPerBlock %>];
                <% end %>
              <% end %>
    barrier(CLK_LOCAL_MEM_FENCE);
              <% r1.times do |k| %>
    lMemStore[<%= k * threadsPerBlock %>] = a[<%= k %>].y;
              <% end %>
    barrier(CLK_LOCAL_MEM_FENCE);
              <% numIter.times do |k| %>
                <% r2.times do |t| %>
        a[<%= k * r2 + t %>].y = lMemLoad[<%= t * batch_size + k * threadsPerBlock %>];
                <% end %>
              <% end %>
    barrier(CLK_LOCAL_MEM_FENCE);
              <% numIter.times do |k| %>
    fftKernel<%= r2 %>(a + <%= k * r2 %>, dir);
              <% end %>
            <% end %>  
    // twiddle
            <% if i < pass_number - 1 %>
    l = ((bNum << <%= lgBatchSize%>) + i) >> <%= lgStrideO %>;
    k = j << <%= Math.log2(r1 / r2).to_i %>;
    ang1 = dir * (2.0f * M_PI / <%= n %>) * l;
              <% r1.times do |t| %>
      ang = ang1 * (k + <%= (t %r2) * r1 + (t / r2) %>);
      w = (float2)(native_cos(ang), native_sin(ang));
      a[<%= t %>] = complexMul(a[<%= t %>], w);
              <% end %>
            <% end %>
  // store data
            <% if strideO == 1 %>
    lMemStore = sMem + mad24(i, <%= radix + 1 %>, j << <%= Math.log2(r1 / r2).to_i %>);
    lMemLoad = sMem + mad24(tid >> <%= Math.log2(radix).to_i %>, <%= radix + 1 %>, tid & <%= radix - 1 %>);  
              <% numIter.times do |k| %>
                <% r2.times do |t| %>
        lMemStore[<%= k + t * r1 %>] = a[<%= k * r2 + t %>].x;
                <% end %>
              <% end %>
    barrier(CLK_LOCAL_MEM_FENCE);
              <% r1.times do |k| %>
    a[<%= k %>].x = lMemLoad[<%= k * (radix + 1) * (threadsPerBlock / radix) %>]; 
              <% end %>
    barrier(CLK_LOCAL_MEM_FENCE);    
              <% numIter.times do |k| %>
                <% r2.times do |t| %>
        lMemStore[<%= k + t * r1 %>] = a[<%= k * r2 + t %>].y;
                <% end %>
              <% end %>
    barrier(CLK_LOCAL_MEM_FENCE);
              <% r1.times do |k| %>
      a[<%= k %>].y = lMemLoad[<%= k * (radix + 1) * (threadsPerBlock / radix) %>]; 
              <% end %>
    barrier(CLK_LOCAL_MEM_FENCE);
    
    indexOut += tid;
              <% if self.data_format == :split_complex %>
    out_real += indexOut;
    out_image += indexOut;
                <% r1.times do |k| %>
    out_real[<%= k * threadsPerBlock %>] = a[<%= k %>].x;
                <% end %>
                <% r1.times do |k| %>
    out_image[<%= k * threadsPerBlock %>] = a[<%= k %>].y;
                <% end %>              
              <% else %>
    out += indexOut;
                <% r1.times do |k| %>
    out[<%= k * threadsPerBlock %>] = a[<%= k %>];
                <% end %>
              <% end %>
            <% else %>
    indexOut += mad24(j, <%= numIter * strideO %>, i);
              <% if self.data_format == :split_complex %>
    out_real += indexOut;
    out_image += indexOut;
                <% r1.times do |k| %>
    out_real[<%= (k % r2) * r1 + (k / r2) * strideO %>] = a[<%= k %>].x;
                <% end %>
                <% r1.times do |k| %>
    out_image[<%= (k % r2) * r1 + (k / r2) * strideO %>] = a[<%= k %>].y;
                <% end %>              
              <% else %>
    out += indexOut;
                <% r1.times do |k| %>
    out[<%= (k % r2) * r1 + (k / r2) * strideO %>] = a[<%= k %>];
                <% end %>
              <% end %>
            <% end %>
          EOT
          body = ERB.new(tp).result(binding)
          
          kernel[:source] = kernel(kernel[:name], r1, body)
          @kernels << kernel
          
          n /= radix
        end       
        self
      end

    end
  end
end
