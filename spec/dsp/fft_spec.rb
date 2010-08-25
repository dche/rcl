
require File.join(File.dirname(__FILE__), '../spec_helper')
require File.join(File.dirname(__FILE__), '../../lib/opencl/dsp/fft')

include OpenCL
include OpenCL::DSP

describe FFT do
  the '#new should reject invalid shape' do
    should.raise(ArgumentError) { FFT.new [2, 2, 4, 4] }
    should.raise(ArgumentError) { FFT.new [nil, 2048] }
    should.raise(ArgumentError) { FFT.new [1] }
    should.raise(ArgumentError) { FFT.new ['64', '64', '64'] }
    should.not.raise(Exception) { FFT.new [1024] }
    should.not.raise(Exception) { FFT.new [8, 8, 64] }
    should.not.raise(Exception) { FFT.new [4096, 64] }
  end
  
  the '::new should accept all valid shapes' do
    ptwo = 1.upto(12).map do |n|; 2 ** n; end
    
    should.not.raise(Exception) do
      # 1D
      ptwo.each do |ar|; FFT.new [ar]; end
      # 2D
      ptwo.permutation(2).to_a.shuffle.take(12).each do |ar|; FFT.new ar; end
      # 3D
      ptwo.permutation(3).to_a.shuffle.take(12).each do |ar|; FFT.new ar; end
      
      FFT.new [1048576]
    end
  end
  
  the '::new should reject invalid data_format' do
    should.raise(ArgumentError) { FFT.new [1024], :no_such_format }
    should.not.raise(Exception) { FFT.new [1024], :split_complex }
    should.not.raise(Exception) { FFT.new [1024], :interleaved_complex }
  end
  
  the '::new should accept custom devcie options' do
    opts = {:max_radix => 64, :max_workgroup_size => 128,
      :max_workitem_size => 512, :max_local_fft_size => 4096, :mem_coalesce_width => 32,
      :local_mem_banks_number => 64}
    
    should.not.raise(Exception) { FFT.new [1024], :interleaved_complex, opts }
    fft = FFT.new [1024, 128], :split_complex, opts
    fft.instance_variable_get(:@max_radix).should.equal 64
    fft.instance_variable_get(:@max_workgroup_size).should.equal 128
    fft.instance_variable_get(:@max_workitem_size).should.equal 512
    fft.instance_variable_get(:@max_local_fft_size).should.equal 4096
    fft.instance_variable_get(:@mem_coalesce_width).should.equal 32
    fft.instance_variable_get(:@local_mem_banks_number).should.equal 64
  end
  
  the 'shape property should be frozen' do
    fft = FFT.new [64, 64, 64]
    fft.shape.should.equal [64, 64, 64]
    fft.shape.should.be.frozen
  end
  
end
