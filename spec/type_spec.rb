
require File.join(File.dirname(__FILE__), 'spec_helper')

describe Type do

  it 'should reject invalid type tag' do
    should.raise(ArgumentError) { Type.new :cl_short7 }
    should.raise(ArgumentError) { Type.new 1234 }
    should.raise(ArgumentError) { Type.new 'cl_int' }
    should.raise(ArgumentError) { Type.new 'name' => [:cl_uchar, 10] }
  end

  describe 'Built-in Type' do
    the '#exact & #inexact?' do
      Type.new(:cl_float).should.be.inexact
      Type.new(:cl_float).should.not.be.exact
      Type.new(:cl_float2).should.be.inexact
      Type.new(:cl_half).should.be.inexact
      Type.new(:cl_uchar4).should.be.exact
      Type.new(:cl_uint).should.not.be.inexact
      Type.new(:cl_long16).should.be.exact
      Type.new(:cl_char).should.be.exact
    end

    it 'should be a number' do
      Type.new(:cl_float8).should.be.number
      Type.new(:cl_uchar).should.be.number
      Type.new(:cl_long16).should.be.number
      Type.new(:cl_half).should.be.number
    end

    the '#compatible?' do
      Type.new(:cl_float).should.be.compatible(:any)
      Type.new(:cl_float).should.be.compatible(:number)
      Type.new(:cl_float).should.be.compatible(:scalar)
      Type.new(:cl_float).should.be.compatible(:inexact)
      Type.new(:cl_float).should.not.be.compatible(:vector)
      Type.new(:cl_float).should.not.be.compatible(:structure)
      Type.new(:cl_ushort2).should.be.compatible(:any)
      Type.new(:cl_ushort2).should.be.compatible(:number)
      Type.new(:cl_ushort2).should.be.compatible(:vector)
      Type.new(:cl_ushort2).should.not.be.compatible(:scalar)
      Type.new(:cl_ushort2).should.not.be.compatible(:structure)
      Type.new(:cl_ushort2).should.be.compatible(:exact)
    end

    the '#size' do
      [:cl_int, :cl_uint2, :cl_long8, :cl_half, :cl_float4].each do |t|
        Type.new(t).size.should.equal OpenCL.type_size(t)
      end
    end
  end

  describe 'Structured Type' do
    the 'basic good case' do
      should.not.raise(Exception) {
        Type.new ['name', :cl_uchar, 10]
        Type.new ['name', :cl_uchar, 10, 'age', :cl_uint]
      }
    end

    it 'should reject invalid field name.' do
      should.raise(ArgumentError) {
        Type.new [:name, :cl_uchar, 10]
      }
      should.raise(ArgumentError) {
        Type.new ['12abc', :cl_uchar, 10]
      }
      should.raise(ArgumentError) {
        Type.new ['ab^&', :cl_float]
      }
    end

    it 'should reject invalid type tag of field.' do
      should.raise(ArgumentError) {
        Type.new ['abc', :cl_uchar5, 10]
      }
      should.raise(ArgumentError) {
        Type.new ['name', ['embedded', :cl_uchar, 10], 'other', :cl_float, 2]
      }
    end

    the '#size' do
      Type.new(['name', :cl_uchar, 10]).size.should.equal 10
      Type.new(['number', :cl_float, 'type', :cl_int]).size.should.equal 8
    end
  end

end
