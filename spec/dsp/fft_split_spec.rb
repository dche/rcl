require_relative 'fft_spec_helper'

describe FFT do
  the 'Inverse of forward FFT should be the waveform itself (after scale).' do
    valid_fft_shapes(2).each do |shape|
      sz = shape.reduce(:*) * (Kernel.rand(16) + 1)  # shape size * batch size
      next if sz > 2e6  # ignore sizes that are too large
      
      fft = FFT.new shape, :split_complex
      
      data_in_real = HostPointer.new :cl_float, sz
      data_out_real = HostPointer.new :cl_float, sz
      buff_in_real = Buffer.new sz * OpenCL.type_size(:cl_float)
      buff_out_real = Buffer.new sz * OpenCL.type_size(:cl_float)
  
      data_in_image = HostPointer.new :cl_float, sz
      data_out_image = HostPointer.new :cl_float, sz
      buff_in_image = Buffer.new sz * OpenCL.type_size(:cl_float)
      buff_out_image = Buffer.new sz * OpenCL.type_size(:cl_float)
      
      sz.times do |i|
        data_in_real[i] = Kernel.rand
        data_in_image[i] = Kernel.rand
      end
  
      buff_in_real.get_data_from data_in_real
      buff_in_image.get_data_from data_in_image
      fft.forward buff_in_real, buff_in_image, buff_out_real, buff_out_image
  
      # clear buffer_in
      buff_in_real.get_data_from data_out_real
      buff_in_image.get_data_from data_out_image
  
      fft.inverse buff_out_real, buff_out_image, buff_in_real, buff_in_image
      buff_in_real.store_data_to data_out_real
      buff_in_image.store_data_to data_out_image
      
      scale = shape.reduce(:*)
      5.times { 
        i = Kernel.rand sz
        data_in_real[i].should.close(data_out_real[i] / scale, 1e-5)
        data_in_image[i].should.close(data_out_image[i] / scale, 1e-5) 
      }
    end
  end
  
  the 'Real time-domain waveform should be symmetry around N/2 in freq-domain, and vice versa.' do
    (4..20).each do |i|
      shape = [2 ** i]
      
      sz = shape.reduce(:*)
      fft = FFT.new shape, :split_complex
  
      data_real = HostPointer.new :cl_float, sz
      data_image = HostPointer.new :cl_float, sz
      buff_real = Buffer.new sz * OpenCL.type_size(:cl_float)
      buff_image = Buffer.new sz * OpenCL.type_size(:cl_float)
      
      sz.times do |i|
        data_real[i] = Kernel.rand
      end
      buff_real.get_data_from data_real
      buff_image.get_data_from data_image
      
      data_real.clear
      fft.forward! buff_real, buff_image
      
      buff_real.store_data_to data_real
      buff_image.store_data_to data_image
      n = fft.shape.x      
      
      10.times do 
        i = Kernel.rand(n / 2 - 1)
        next if i == 0
  
        data_real[i].should.close data_real[n - i], 1e-3
        data_image[i].should.close -data_image[n - i], 1e-3
      end
    end
  end
  
  the 'FFT of DC waveform should be the DC value.' do
    valid_fft_shapes(5).each do |shape|
      sz = shape.reduce(:*) * (Kernel.rand(256) + 1)
      next if sz > 2e6

      fft = FFT.new shape, :split_complex
      
      data_real = HostPointer.new :cl_float, sz
      buff_in_real = Buffer.new sz * OpenCL.type_size(:cl_float)
      buff_out_real = Buffer.new sz * OpenCL.type_size(:cl_float)
      data_image = HostPointer.new :cl_float, sz
      buff_in_image = Buffer.new sz * OpenCL.type_size(:cl_float)
      buff_out_image = Buffer.new sz * OpenCL.type_size(:cl_float)
      
      sz.times do |i|
        data_real[i] = 1.0
      end
      data_image.clear
  
      buff_in_real.get_data_from data_real
      buff_in_image.get_data_from data_image
      fft.forward buff_in_real, buff_in_image, buff_out_real, buff_out_image
      
      data_real.clear
      
      buff_out_real.store_data_to data_real
      buff_out_image.store_data_to data_image
      
      ssz = shape.reduce(:*)
      bsz = sz / ssz
      5.times do
        i = Kernel.rand ssz
        j = Kernel.rand bsz
        data_real[j * ssz].should.equal ssz
        data_image[j * ssz + i].should.equal 0 if i > 0
      end
    end
  end
  
end
