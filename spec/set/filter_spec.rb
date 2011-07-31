# encoding: utf-8

require File.join(File.dirname(__FILE__), '../spec_helper')
require File.join(File.dirname(__FILE__), '../../lib/opencl/set')

class TestSet < OpenCL::Set
  def initialize
    super ['id', :cl_uint, 'value', :cl_float]
  end

  lib = Class.new(SetLibrary) do
    type ['id', :cl_uint, 'value', :cl_float]

    def_map_kernel(:rcl_set_spec_map, :zero_to_zero) do |elm, keep, dirty|
      <<-EOM
if (#{elm}.id == 0) {
   #{elm}.value = 0;
} else {
   #{elm}.value = 2.125;
}
#{dirty} = true;
      EOM
    end

    def_map_kernel(:rcl_set_spec_filter, :nonzero_to_go) do |elm, keep, dirty|
      <<-EOM
if (#{elm}.id != 0) {
    #{keep} = false;
}
      EOM
    end
  end
  use lib
end

class GCSet < OpenCL::Set
  def initialize
    super :cl_uint
  end

  lib = Class.new(OpenCL::SetLibrary) do
    type :cl_uint

    def_gc_kernel do |elm|
      "#{elm} == 0"
    end
  end
  use lib
end

describe Set do
  it 'should use SetLibrary only.' do
    lib = Class.new(Library) do; end
    should.raise(TypeError) { TestSet.use(lib) }
  end

  it 'should not respond to :reduce' do
    set = Set.new :cl_float
    set.should.not.respond_to :reduce
  end

  the 'map operation' do
    set = TestSet.new
    set.should.respond_to :zero_to_zero
    st = Structure.new set.type.tag
    st.id = 0
    st.value = 1.0
    i = set.put st
    st.id = 1
    st.value = 1.0
    j = set.put st

    set[i].id.should.equal 0
    set[i].value.should.equal 1.0
    set[j].id.should.equal 1
    set[j].value.should.equal 1.0
    set.zero_to_zero
    set[i].id.should.equal 0
    set[i].value.should.equal 0
    set[j].id.should.equal 1
    set[j].value.should.equal 2.125

    set.should.respond_to :nonzero_to_go
    set.count.should.equal 2
    set.nonzero_to_go
    set[j].should.be.nil
    set[i].id.should.equal 0
    set[i].value.should.equal 0
    set.count.should.equal 1
  end

  the "#gc" do
    set = GCSet.new
    set.should.be.respond_to :gc

    ct = set.length
    # force trigger gc.
    set.length.times do |n|
      set.put(n % 2)
    end
    # elements that equal 0 are collected.
    (set.count < ct).should.be.true
  end
end
