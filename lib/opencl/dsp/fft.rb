#
# copyright (c) 2010, Che Kenan
#
#--
# The FFT class is adapted from Apple's FFT OpenCL_FFT sample code.
#++
#

require_relative 'fft_kernel_templates'

module OpenCL
  module DSP

    # An FFT object is an execution plan to compute certain type of FFT.
    #
    # To convert data between time domain and frequency domain, you first
    # create a FFT object that fits the characteristics of your data, 
    # and then call the +forward()+ or +inverse()+ method.
    #
    # For real FFT, 
    #
    # Examples:
    # 
    #   # Create FFT execution plan for 1D, interleaved data.
    #   fft1d = FFT.new [1024], :interleaved_complex
    #
    #   fft1d.forward inBuffer, outBuffer
    #   fft1d.inverse! outBuffer
    #
    class FFT
      include FFTKernelTemplates
      
      attr_reader :shape
      attr_reader :data_format
      
      @@base_src = File.open(File.join(File.dirname(__FILE__), 'fft_base.cl')) do |f|
        f.read
      end
      
      # Create an FFT execution plan.
      def initialize(shape, data_format = :interleaved_complex, opts = {})
        verify_shape shape
        verify_data_format data_format
        verify_device_options opts
        
        @program = OpenCL::Program.new
        
        max_workgroup_size = @max_workgroup_size
        begin
          @max_workgroup_size = max_workgroup_size
          
          @kernels = []
          generate_kernels

          source = @@base_src + @kernels.map do |k|
            k[:source]
          end.reduce(:+)
          
          begin  
            @program.compile source, '-cl-mad-enable'
          rescue OpenCL::ProgramBuildError => e
            warn e.message
            warn 'This might be cause by wrong settings of program options.'
            warn 'Use smaller values and try again.'
            break
          end
          
          max_workgroup_size = @program.max_workgroup_size
        end while @max_workgroup_size > max_workgroup_size
        
        # define methods according to data_format
        case self.data_format
        when :interleaved_complex
          class <<self
            def forward(in_buffer, out_buffer)
              execute_interleaved(in_buffer, out_buffer, :forward)
            end
            
            def forward!(buffer)
              self.forward buffer, buffer
            end
            
            def inverse(in_buffer, out_buffer)
              execute_interleaved(in_buffer, out_buffer, :inverse)
            end
            
            def inverse!(buffer)
              self.inverse buffer, buffer
            end
          end
        when :split_complex
          class <<self
            def forward(in_real, in_image, out_real, out_image)              
              execute_split(in_real, in_image, out_real, out_image, :forward)
            end
            
            def forward!(real, image)
              self.forward(real, image, real, image)
            end
            
            def inverse(in_real, in_image, out_real, out_image)
              execute_split(in_real, in_image, out_real, out_image, :inverse)
            end
            
            def inverse!(real, image)
              self.inverse(real, image, real, image)
            end
          end
        end
      end
      
      # Returns the generated OpenCL source code.
      def source
        @program.source
      end

      private
      
      def verify_shape(shape)
        x, y, z = shape.map do |v|; v.nil? ? 1 : v; end

        if shape.length > 3 || x == 1 || (y == 1 && shape.length != 1) || (y != 1 && z == 1 && shape.length != 2)
          raise ArgumentError, 'Invalid shape.'
        end
        
        shape.each do |val|
          break if val.nil?
          
          if !val.is_a?(Fixnum) || val < 1
            raise ArgumentError, 'Size must be a positive Integer.'
          end
        end
        
        @shape = shape.dup
        if 6 * OpenCL.type_size(:cl_float2) * shape.reduce(:*) > OpenCL::Context.default_context.max_mem_alloc_size
          raise ArgumentError, "Shape size is too large to fit the OpenCL devices in this platform."
        end
        
        def @shape.x; self[0]; end
        def @shape.y; self[1] || 1; end
        def @shape.z; self[2] || 1; end
        
        @shape.freeze
        self
      end
      
      def verify_data_format(format)
        case format
        when :interleaved_complex, :split_complex
          @data_format = format
        else
          raise ArgumentError, "Invalid data format: #{format}"
        end
        self
      end
      
      def verify_device_options(opts)
        {
          'max_radix' => 16,
          'max_workgroup_size' => 256,
          'max_workitem_size' => 256,
          'max_local_fft_size' => 2048,
          'mem_coalesce_width' => 16,
          'local_mem_banks_number' => 16,
        }.each do |k, v|
          val = opts[k.to_sym] || v
          if !val.is_a?(Fixnum) || val < 1
            raise TypeError, "Invalid option value for #{k}. Expected a positive Integer."
          end
          
          self.instance_variable_set ('@' + k).to_sym, val
        end
        
        # prevent max_radix <= 1
        @max_radix = 8 if @max_radix < 2
        # TODO: other restrictions to be added here.
        self
      end
      
      def generate_kernels
        case self.shape.size
        when 1
          fft_1d(:x)
        when 2
          fft_1d(:x); fft_1d(:y)
        when 3
          fft_1d(:x); fft_1d(:y); fft_1d(:z)
        end
      end
      
      def fft_1d(axis)
        case axis
        when :x
          x = @shape.x
          if x > @max_local_fft_size
            global_fft_kernel_string x, 1, :x, 1
          else
            ra = radix_array(x, 0)
            if ra.empty?
              global_fft_kernel_string x, 1, :x, 1 
            elsif x / ra.first <= @max_workitem_size
              local_fft_kernel_string(x, ra)
            else
              ra = radix_array(x, @max_radix)
              if x / ra.first <= @max_workitem_size
                local_fft_kernel_string(x, ra)
              else
                global_fft_kernel_string x, 1, :x, 1
              end
            end
          end
        when :y
          global_fft_kernel_string @shape.y, @shape.x, :y, 1
        when :z
          global_fft_kernel_string @shape.z, @shape.x * @shape.y, :z, 1
        end
      end
      
      # Compute the radix array.
      def radix_array(n, max_radix)        
        if max_radix > 1  
          max_radix = [n, max_radix].min
          ra = []

          while n > max_radix
            ra << max_radix
            n /= max_radix
          end
          
          ra << n
        else
          case n
          when 2, 4, 8
            [n]
          when 16
            [8, 2]
          when 32
            [8, 4]
          when 64
            [8, 8]
          when 128
            [8, 4, 4]
          when 256
            [4, 4, 4, 4]
          when 512
            [8, 8, 8]
          when 1024
            [16, 16, 4]
          when 2048 # == max_local_fft_size 
            [8, 8, 8, 4]
          else
            []
          end
        end
      end
      
      def need_temp_buffer?
        @inplace_possible ||= @kernels.map do |k|; k[:inplace_possible]; end.inject(:&)
        !@inplace_possible
      end
      
      def execute_interleaved(in_buff, out_buff, direction)
        return self unless self.data_format == :interleaved_complex

        # sanity checks
        if in_buff.byte_size != out_buff.byte_size
          raise ArgumentError, "Sizes of input and output buffers mismatch (#{in_buff.byte_size} and #{out_buff.byte_size})."
        end
        
        in_place = (in_buff == out_buff)
        
        # compute batch size
        unit_size = self.shape.reduce(:*) * OpenCL.type_size(:cl_float2)
        batch_size = in_buff.byte_size / unit_size
        if batch_size < 1
          raise ArgumentError, 
                "Buffer sizes are too small, at least #{unit_size} bytes are needed."
        end
     
        alloc_interleaved_temp_buffer in_buff.byte_size
        # @temp_buffer might be nil, but that's OK.
        mem_objs = [in_buff, out_buff, @temp_buffer]
        curr_read = 0
        curr_write = 1
        
        dir = (direction == :forward ? -1 : 1)
        if need_temp_buffer?
          in_place_done = true
          
          if in_place
            in_place_done = false
            curr_read = 1; curr_write = 2
          else
            # ensure the last write target is output buffer.
            curr_write = (@kernels.length.odd?) ? 1 : 2
          end
          
          @kernels.each do |kn|
            if in_place && @kernels.length.odd? && !in_place_done && kn[:inplace_possible]
              curr_write = curr_read
              in_place_done = true
            end

            gws, lws, s = kernel_work_dimensions kn, batch_size
            @program.call kn[:name], [[gws], [lws]],
                          :mem, mem_objs[curr_read],
                          :mem, mem_objs[curr_write],
                          :cl_int, dir,
                          :cl_int, s
            
            curr_read = (curr_write == 1) ? 1 : 2
            curr_write = (curr_write == 1) ? 2 : 1
          end
        else
          @kernels.each do |kn|
            gws, lws, s = kernel_work_dimensions kn, batch_size
            @program.call kn[:name], [[gws], [lws]], 
                          :mem, mem_objs[curr_read],
                          :mem, mem_objs[curr_write],
                          :cl_int, dir,
                          :cl_int, s
            
            curr_read = 1
          end
        end
        self
      end
      
      def execute_split(in_real, in_image, out_real, out_image, direction)
        return self unless self.data_format == :split_complex
        
        if (in_real.byte_size != in_image.byte_size) || (out_real.byte_size != out_image.byte_size) || (in_real.byte_size != out_real.byte_size)
          raise ArgumentError, "Sizes of buffers mismatch."
        end
        
        if (in_real == out_real) != (in_image == out_image)
          raise ArgumentError, "Buffers are overlapped."
        end
        
        in_place = (in_real == out_real)        
        dir = (direction == :forward ? -1 : 1)
        
        size = in_real.byte_size
        unit_size = (self.shape.reduce(:*) * OpenCL.type_size(:cl_float))
        batch_size = size / unit_size
        if batch_size < 1
          raise ArgumentError, 
                "Buffer sizes are too small to fit the shape, at least #{unit_size} bytes are needed."
        end

        alloc_split_temp_buffer(size)        
        mem_objs_real = [in_real, out_real, @temp_buffer_real]
        mem_objs_image = [in_image, out_image, @temp_buffer_image]
        curr_read = 0
        curr_write = 1
        
        if need_temp_buffer?
          in_place_done = true
          
          if in_place
            in_place_done = false
            curr_read = 1; curr_write = 2
          else
            # ensure last write target is output buffer.
            curr_write = (@kernels.length.odd?) ? 1 : 2
          end
          
          @kernels.each do |kn|
            if in_place && @kernels.length.odd? && !in_place_done && kn[:inplace_possible]
              curr_write = curr_read
              in_place_done = true
            end

            gws, lws, s = kernel_work_dimensions kn, batch_size
            @program.call kn[:name], [[gws], [lws]],
                          :mem, mem_objs_real[curr_read],
                          :mem, mem_objs_image[curr_read],
                          :mem, mem_objs_real[curr_write],
                          :mem, mem_objs_image[curr_write],
                          :cl_int, dir,
                          :cl_int, s
            
            curr_read = (curr_write == 1) ? 1 : 2
            curr_write = (curr_write == 1) ? 2 : 1            
          end
        else
          @kernels.each do |kn|
            gws, lws, s = kernel_work_dimensions kn, batch_size
            @program.call kn[:name], [[gws], [lws]],
                          :mem, mem_objs_real[curr_read],
                          :mem, mem_objs_image[curr_read],
                          :mem, mem_objs_real[curr_write],
                          :mem, mem_objs_image[curr_write],
                          :cl_int, dir,
                          :cl_int, s
            
            curr_read = 1
          end
        end
        self
      end
      
      def alloc_interleaved_temp_buffer(size)
        if need_temp_buffer? && self.data_format == :interleaved_complex
           unless @temp_buffer && @temp_buffer.byte_size >= size
             @temp_buffer = OpenCL::Buffer.new size
           end
        end
        self
      end
      
      def alloc_split_temp_buffer(size)
        if need_temp_buffer? && self.data_format == :split_complex
          unless @temp_buffer_real && @temp_buffer_real.byte_size >= size
             @temp_buffer_real = OpenCL::Buffer.new size
             @temp_buffer_image = OpenCL::Buffer.new size
          end
        end
        self
      end
      
      # Returns an Array of global work size and local work size.
      def kernel_work_dimensions(kernel, batch_size)
        lws = kernel[:num_workitem_per_workgroup]
        numWorkGroups = kernel[:num_workgroup]
        numXFormsPerWG = kernel[:num_xforms_per_workgroup]
        
        case kernel[:axis]
        when :x
          batch_size *= self.shape.y * self.shape.z
          numWorkGroups *= (batch_size % numXFormsPerWG != 0) ? (batch_size / numXFormsPerWG + 1) : (batch_size / numXFormsPerWG)
        when :y
          batch_size *= self.shape.z
          numWorkGroups *= batch_size
        when :z
          numWorkGroups *= batch_size
        end
        [numWorkGroups * lws, lws, batch_size]
      end
    end
  end  
end
