
require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL::Capi

describe 'Half precision float conversion' do
  before do
    @p = HostPointer.new :cl_half, 1
  end
  
  the "zero." do
    @p = HostPointer.new :cl_half, 1
    @p[0] = 0
    @p[0].should.equal 0
  end
  
  the 'NaN' do
    @p[0] = 0.0/0.0
    @p[0].should.is_a Float
    @p[0].should.be.nan
  end
  
  the 'Infinite' do
    @p[0] = 1.0/0.0
    @p[0].should.is_a Float
    @p[0].infinite?.should.not.be.nil
  end
  
  the 'small numbers' do
    @p[0] = -2
    @p[0].should.equal -2
  end
  
  the 'edge values' do
    @p[0] = 65504
    @p[0].should.equal 65504
    
    minimum_positive_normal = 6.10352e-5
    @p[0] = minimum_positive_normal
    ((@p[0] - minimum_positive_normal).abs < 1e-4).should.be.true
    
    minimum_strictly_positive_subnormal = 5.96046e-8
    @p[0] = minimum_strictly_positive_subnormal
    ((@p[0] - minimum_strictly_positive_subnormal).abs < 1e-4).should.be.true
    
  end
end
