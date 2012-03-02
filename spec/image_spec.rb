# encoding: utf-8

require File.join(File.dirname(__FILE__), 'spec_helper')

describe Image do
  describe '#new' do
    it 'should reject invalid sizes.' do
      should.raise(ArgumentError) { Image.new [10, 10, 0] }
      should.raise(ArgumentError) { Image.new [10, -10] }
      should.raise(Exception) { Image.new ["a", 100] }
      should.raise(ArgumentError) { Image.new [10, 1, 10] }
      should.not.raise(ArgumentError) { Image.new [1, 20, 20] }
      should.not.raise(Exception) { Image.new [10, 1, 1] }
      should.not.raise(Exception) { Image.new [10, 20, 1] }
    end

    it 'should.reject invalid image format.' do
      should.raise(ArgumentError) { Image.new [100, 200], format:[CL_BGRA, CL_FLOAT] }
      should.raise(ArgumentError) { Image.new [100, 200, 300], format:"CL_RGBA, CL_FLOAT" }
    end

    it 'should reject data that has wrong size.' do
      ptr = HostPointer.new :cl_float4, 100
      should.raise(ArgumentError) { Image.new [10, 10], data:"Pointer" }
      should.raise(ArgumentError) { Image.new [10, 10], format:[CL_RGBA, CL_UNSIGNED_INT8], data:ptr }
      should.raise(ArgumentError) { Image.new [100, 10], data:ptr }
    end

    it 'should reject wrong context.' do
      should.raise(ArgumentError) { Image.new [10, 10], context:"Context" }
    end

    the 'constructor' do
      img = Image.new [100, 200, 1], format:[CL_R, CL_FLOAT]
      img.memory.should.is_a Capi::Memory
      img.memory.image_info(CL_IMAGE_WIDTH).should.equal 100
      img.memory.image_info(CL_IMAGE_HEIGHT).should.equal 200
      img.memory.image_info(CL_IMAGE_ELEMENT_SIZE).should.equal 4
      img.memory.image_info(CL_IMAGE_ROW_PITCH).should.equal 100 * 4
    end

    the '#dup' do
      img = Image.new [100]
      dp = img.dup
      dp.width.should.equal 100
      dp.dimension.should.equal 1
      dp.memory.should.not.equal img.memory
      img.memory.image_info(CL_IMAGE_WIDTH).should.equal 100
      img.memory.image_info(CL_IMAGE_ELEMENT_SIZE).should.equal 4
    end

    the '#byte_size' do
      img = Image.new [10, 10], format:[CL_RGBA, CL_FLOAT]
      img.byte_size.should.equal (4 * 4 * 10 * 10)
    end

    the '#format' do
      Image.new([10, 10], format:[CL_RGBA, CL_FLOAT]).format.should.equal [CL_RGBA, CL_FLOAT]
    end

    the '#width, #height, #depth' do
      img = Image.new [10]
      img.width.should.equal 10
      img.height.should.equal 1
      img.depth.should.equal 1
      img = Image.new [10, 20, 1]
      img.width.should.equal 10
      img.height.should.equal 20
      img.depth.should.equal 1
      img = Image.new [30, 20, 10]
      img.width.should.equal 30
      img.height.should.equal 20
      img.depth.should.equal 10
    end

    the '#dimension' do
      img = Image.new [10]
      img.dimension.should.equal 1
      img.should.be.oned
      img.should.not.be.twod
      img.should.not.be.threed
      img = Image.new [10, 20]
      img.dimension.should.equal 2
      img.should.not.be.oned
      img.should.be.twod
      img.should.not.be.threed
      img = Image.new [10, 20, 30]
      img.dimension.should.equal 3
      img.should.not.be.oned
      img.should.not.be.twod
      img.should.be.threed
    end

    the '#read and #write' do
      ptr = HostPointer.new :cl_float4, 200
      ptr[0] = [0.125, 0.25, 0.375, 0.5]
      ptr[199] = [0.5, 0.375, 0.25, 0.125]
      img = Image.new [10, 20], format:[CL_RGBA, CL_FLOAT]
      img.write ptr
      ptr.clear
      ptr[0].should.equal [0.0, 0.0, 0.0, 0.0]
      ptr[199].should.equal [0.0, 0.0, 0.0, 0.0]
      img.read ptr
      ptr[0].should.equal [0.125, 0.25, 0.375, 0.5]
      ptr[199].should.equal [0.5, 0.375, 0.25, 0.125]
    end

    # the '#copy' do
    # end
    #
    # the '#resize' do
    # end
  end
end
