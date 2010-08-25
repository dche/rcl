#
# copyright (c) 2010, Che Kenan
#
#--
# The FFT class is adapted from Apple's FFT OpenCL_FFT sample code.
#++
#

require File.join(File.dirname(__FILE__), 'fft_kernel_templates')

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
    #   fft1d = FFT.new [1024], :interleaved
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
      
      # Create an FFT execution plan for the combination of data size and 
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

          @program.compile @@base_src + @kernels.map do |k|
            k[:soruce]
          end.reduce(:+)
          
          max_workgroup_size = @program.max_workgroup_size
        end while @max_workgroup_size > max_workgroup_size
      end
      
      def kernel_source
        @program.source
      end

      # Executes the FFT plan for given samples
      def forward(in_buffer, out_buffer)
      end
      
      def forward!(buffer)
      end

      def inverse(in_buffer, out_buffer)
      end
      
      def inverse!(buffer)
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
        
        def @shape.x; self[0]; end
        def @shape.y; self[1]; end if y
        def @shape.z; self[2]; end if z
        
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
            if x / ra.first <= @max_workitem_size
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
            [4, 4]
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
          when 2048
            [8, 8, 8, 4]
          else
            []
          end
        end
      end

    end
  end  
end
