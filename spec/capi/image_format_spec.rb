
require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL::Capi

describe ImageFormat do

  the "constructor" do
    should.raise(TypeError) { ImageFormat.new("a", 2) }
    should.not.raise(Exception) { ImageFormat.new(1, 2) }
  end

  the "supported image formats" do
    cxt = OpenCL::Capi::Context.new([CL_CONTEXT_PLATFORM, platforms.first], CL_DEVICE_TYPE_ALL)
    mem_flags = CL_MEM_WRITE_ONLY
    formats = cxt.supported_image_formats(mem_flags, CL_MEM_OBJECT_IMAGE2D)

    formats.should.is_a Array
    formats.first.should.is_a ImageFormat
    (formats.first.channel_order != 0).should.be.true
    (formats.last.channel_data_type > 0).should.be.true
  end
end
