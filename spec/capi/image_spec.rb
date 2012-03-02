
require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL::Capi

IMAGE_FORMAT = ImageFormat.new(CL_BGRA, CL_UNORM_INT8)
CXT = OpenCL::Capi::Context.new(platforms.first, CL_DEVICE_TYPE_ALL)

describe "2D and 3D Image" do
  the 'create_image_2d()' do
    img = CXT.create_image_2d(CL_MEM_WRITE_ONLY, IMAGE_FORMAT, 200, 200, 0, nil)
    img.should.is_a Memory
    img.info(CL_MEM_TYPE).should.equal CL_MEM_OBJECT_IMAGE2D
    img.image_info(CL_IMAGE_WIDTH).should.equal 200
    img.image_info(CL_IMAGE_HEIGHT).should.equal 200
  end

  the 'create_image_3d()' do
    # Must be read only for 3d iamge?
    img = CXT.create_image_3d(CL_MEM_READ_ONLY, IMAGE_FORMAT, 800, 800, 2, 0, 0, nil)
    img.should.is_a Memory
    img.info(CL_MEM_TYPE).should.equal CL_MEM_OBJECT_IMAGE3D
  end
end
