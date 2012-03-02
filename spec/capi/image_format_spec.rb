
require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL::Capi

describe ImageFormat do

  the "constructor" do
    should.raise(TypeError) { ImageFormat.new("a", 2) }
    should.raise(ArgumentError) { ImageFormat.new(1, 2) }
    should.raise(ArgumentError) { ImageFormat.new CL_BGRA, CL_FLOAT }
    should.not.raise(Exception) { ImageFormat.new CL_R, CL_FLOAT }
  end

  the '#channel_order' do
    ImageFormat.new(CL_R, CL_FLOAT).channel_order.should.equal CL_R
  end

  the '#channel_data_type' do
    ImageFormat.new(CL_RGBA, CL_FLOAT).channel_data_type.should.equal CL_FLOAT
  end

  the '#element_size' do
    ImageFormat.new(CL_R, CL_FLOAT).element_size.should.equal 4
    ImageFormat.new(CL_BGRA, CL_UNORM_INT8).element_size.should.equal 4
    ImageFormat.new(CL_RGBA, CL_FLOAT).element_size.should.equal 16
  end

  the "Context#supported_image_formats" do
    cxt = OpenCL::Capi::Context.new([CL_CONTEXT_PLATFORM, platforms.first], CL_DEVICE_TYPE_ALL)
    mem_flags = CL_MEM_WRITE_ONLY
    formats = cxt.supported_image_formats(mem_flags, CL_MEM_OBJECT_IMAGE2D)

    formats.should.is_a Array
    formats.first.should.is_a ImageFormat
    (formats.first.channel_order != 0).should.be.true
    (formats.last.channel_data_type > 0).should.be.true
  end
end
