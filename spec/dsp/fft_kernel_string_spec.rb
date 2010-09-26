
require_relative 'fft_spec_helper'

describe FFT do
  the '::new should reject invalid shape' do
    should.raise(ArgumentError) { FFT.new [2, 2, 4, 4] }
    should.raise(ArgumentError) { FFT.new [nil, 2048] }
    should.raise(ArgumentError) { FFT.new [1] }
    should.raise(ArgumentError) { FFT.new [256, 1] }
    should.raise(ArgumentError) { FFT.new [2, 1, 8] }
    should.raise(ArgumentError) { FFT.new ['64', '64', '64'] }
    should.not.raise(Exception) { FFT.new [1024] }
    should.not.raise(Exception) { FFT.new [8, 8, 64] }
    should.not.raise(Exception) { FFT.new [4096, 64] }
  end
  
  the '::new should reject shape that is too large.' do
    should.raise(ArgumentError) { FFT.new [4096, 4096] }
  end
  
  the '::new should accept all valid shapes' do    
    valid_fft_shapes(10).each do |shp|
      should.not.raise(Exception) do
        FFT.new shp
      end
    end
    
    should.not.raise(Exception) do
      FFT.new [1048576, 2]
    end
  end
    
  the '::new should reject invalid data_format' do
    should.raise(ArgumentError) { FFT.new [1024], :no_such_format }
    should.not.raise(Exception) { FFT.new [1024], :split_complex }
    should.not.raise(Exception) { FFT.new [1024], :interleaved_complex }
  end
  
  the '::new should accept custom devcie options' do
    opts = {:max_radix => 64, :max_workgroup_size => 128,
      :max_workitem_size => 128, :max_local_fft_size => 4096, :mem_coalesce_width => 8,
      :local_mem_banks_number => 8}
    
    should.not.raise(Exception) { FFT.new [1024, 256], :interleaved_complex, opts }
    fft = FFT.new [4096], :split_complex, opts
    fft.instance_variable_get(:@max_radix).should.equal 64
    fft.instance_variable_get(:@max_workgroup_size).should.equal 128
    fft.instance_variable_get(:@max_workitem_size).should.equal 128
    fft.instance_variable_get(:@max_local_fft_size).should.equal 4096
    fft.instance_variable_get(:@mem_coalesce_width).should.equal 8
    fft.instance_variable_get(:@local_mem_banks_number).should.equal 8
  end
    
  the 'shape property should be frozen' do
    fft = FFT.new [128, 1024]
    fft.shape.should.equal [128, 1024]
    fft.shape.should.be.frozen
  end
end
