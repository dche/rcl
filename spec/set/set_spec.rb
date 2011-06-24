# encoding: utf-8

require File.join(File.dirname(__FILE__), '../spec_helper')
require File.join(File.dirname(__FILE__), '../../lib/opencl/set')

describe Set do
  the "::new" do
    st = Set.new :cl_float
    st.length.should.equal 1024
  end

  given Set do
    the 'initial #count should be zero.' do
      sut = Set.new(:cl_uchar)
      sut.count.should.equal 0
      sut[0].should.be.nil
    end

    the '#[]= should not be able to add element.' do
      sut = Set.new(:cl_uchar)
      sut[0].should.be.nil
      sut[0] = 1
      sut[0].should.be.nil
    end

    the '#length' do
      sut = Set.new :cl_float, capacity:2
      sut.length.should.equal 1024
      sut.byte_size.should.equal 1024 * sut.type.size
    end

    the '#put a scalar' do
      sut = Set.new :cl_uint, capacity:4
      i = sut.put 2
      sut.count.should.equal 1
      sut[i].should.equal 2
      j = sut.put 3
      sut.count.should.equal 2
      sut[j].should.equal 3

      should.raise(TypeError) do
        sut.put 2.0
      end
      sut.count.should.equal 2
    end

    the '#put a vector' do
      sut = Set.new :cl_float4, capacity:16
      i = sut.put [2, 3, 4, 5]
      sut.count.should.equal 1
      sut[i].should.equal [2.0, 3.0, 4.0, 5.0]
    end

    the '#put a structure' do
      tag = ["id", :cl_uint, "value", :cl_float]
      sut = Set.new tag, capacity:32

      sut[0].should.be.nil
      st = Structure.new tag
      st.id = 12345
      st.value = 2.125
      i = sut.put st
      sut.count.should.equal 1
      ot = sut[i]
      ot.is_a?(Structure).should.be.true
      ot.type.should.equal Type.new(tag)
      ot.id.should.equal 12345
      ot.value.should.equal 2.125

      j = sut.put ot
      i.should.not.equal j
      ot = sut[j]
      ot.id.should.equal 12345
      ot.value.should.equal 2.125
    end

    the '#[] should return nil if a cell is not occupied.' do
      sut = Set.new :cl_uint, capacity:4
      sut[0].should.be.nil
    end
  end
end
