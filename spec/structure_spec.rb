
require File.join(File.dirname(__FILE__), 'spec_helper')

tag = ['name', :cl_uchar, 10, 'age', :cl_uint, 'id', :cl_ushort2]

describe Structure do

  it 'should reject invalid arguments' do
    should.raise(ArgumentError) {
      Structure.new :cl_float16
    }

    should.raise(ArgumentError) {
      Structure.new ['name', :cl_uint5, 2]
    }
  end

  it 'should be able to wrap a pointer' do
    sz = Type.new(tag).size
    ptr = HostPointer.new :cl_uchar, sz * 2
    ptr.assign 'john smith'
    sut = Structure.new tag, ptr
    sut.name[0].should.equal 'j'.ord
    sut.name[1].should.equal 'o'.ord
    sut.name[9].should.equal 'h'.ord

    sut.wrap ptr, sz
    ptr.assign 'aiww', sz
    sut.name[0].should.equal 'a'.ord
    sut.name[1].should.equal 'i'.ord
    sut.name[2].should.equal 'w'.ord
  end

  the 'accessor methods' do
    sut = Structure.new tag
    should.not.raise(Exception) {
      sut.name = 'smith john'
      sut.age = 22
      sut.id = [1234, 5678]
      sut.age.should.equal 22
      sut.id.should.equal [1234, 5678]
    }
  end

  given 'wrong number of values' do
    it 'should raise ArgumentError' do
      sut = Structure.new tag
      should.raise(ArgumentError) {
        sut.name = 'joe smith'
      }
      # should.raise(ArgumentError) {
      #   sut.age = 22, 33
      # }
    end
  end

  it 'should be able to create a pointer' do
    sut = Structure.new tag
    sut.age = 22
    sut.age.should.equal 22
  end

  it 'should reject pointer whose size is less than the type size.' do
    ptr = HostPointer.new :cl_uchar, 2
    should.raise(ArgumentError) {
      Structure.new tag, ptr
    }
    ptr = HostPointer.new :cl_uchar, Type.new(tag).size
    should.not.raise(ArgumentError) {
      Structure.new tag, ptr
    }
    should.raise(ArgumentError) {
      Structure.new tag, ptr, 1
    }
  end

  the 'method_missing should work' do
    sut = Structure.new tag
    should.raise(NoMethodError) {
      sut.no_such_method
    }
  end

  the '#size' do
    sut = Structure.new tag
    sut.size.should.equal Type.new(tag).size
  end
end
