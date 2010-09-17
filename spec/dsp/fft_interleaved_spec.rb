require File.join(File.dirname(__FILE__), 'fft_spec_helper')

describe FFT do
  the 'Inverse of forward FFT should be the waveform itself (after scale).' do
    valid_fft_shapes.each do |shape|
      sz = shape.reduce(:*) * (Kernel.rand(16) + 1)  # shape size * batch size
      next if sz > 2e6  # ignore sizes that are too large
      
      fft = FFT.new shape
      
      data_in = HostPointer.new :cl_float2, sz
      data_out = HostPointer.new :cl_float2, sz
      buff_in = Buffer.new sz * OpenCL.type_size(:cl_float2)
      buff_out = Buffer.new sz * OpenCL.type_size(:cl_float2)
      
      sz.times do |i|
        data_in[i] = [Kernel.rand, Kernel.rand]
      end

      buff_in.get_data_from data_in
      fft.forward buff_in, buff_out

      # clear buffer_in
      buff_in.get_data_from data_out

      fft.inverse buff_out, buff_in
      buff_in.store_data_to data_out
      
      scale = shape.reduce(:*)
      5.times { 
        i = Kernel.rand sz
        data_in[i][0].should.close(data_out[i][0] / scale, 1e-5)
        data_in[i][1].should.close(data_out[i][1] / scale, 1e-5) 
      }
    end
  end
  
  the 'Real time-domain waveform should be symmetry around N/2 in freq-domain, and vice versa.' do
    (4..20).each do |i|
      shape = [2 ** i]
      
      sz = shape.reduce(:*)
      fft = FFT.new shape
  
      data = HostPointer.new :cl_float2, sz
      buff = Buffer.new sz * OpenCL.type_size(:cl_float2)
      
      sz.times do |i|
        data[i] = [Kernel.rand, 0]
      end
      buff.get_data_from data
      
      data.clear
      fft.forward! buff
      
      buff.store_data_to data
      n = fft.shape.x
      
      10.times do 
        i = Kernel.rand(n / 2 - 1)
        next if i == 0
  
        data[i][0].should.close data[n - i][0], 1e-3
        data[i][1].should.close -data[n - i][1], 1e-3
      end
    end
  end
  
  the 'FFT of DC waveform should be the DC value.' do
    valid_fft_shapes(5).each do |shape|
      sz = shape.reduce(:*) * (Kernel.rand(256) + 1)
      next if sz > 2e6
      
      fft = FFT.new shape
      
      data = HostPointer.new :cl_float2, sz
      buff_in = Buffer.new sz * OpenCL.type_size(:cl_float2)
      buff_out = Buffer.new sz * OpenCL.type_size(:cl_float2)
      
      sz.times do |i|
        data[i] = [1, 0]
      end
      buff_in.get_data_from data
      fft.forward buff_in, buff_out
      
      data.clear
      
      buff_out.store_data_to data
      data[0].first.should.equal shape.reduce(:*)
    end
  end
  
  it 'should be able to execute many times.' do
  end
  
end
