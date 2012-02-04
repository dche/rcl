
require File.join(File.dirname(__FILE__), '../spec_helper')

include OpenCL::Capi

describe Platform do
  it "should not initialized by new." do
    should.raise(Exception) { Platform.new }
  end

  the 'platforms() should returns an Array of Platform' do
    pfs = platforms
    pfs.size.should.not.equal 0
    pf = pfs.first
    pf.should.is_a Platform
  end

  the 'info query should work.' do
    pf = platforms.first

    pf.should.is_a Platform
    pf.should.respond_to :info

    pf.info(CL_PLATFORM_PROFILE).should.is_a String
    pf.info(CL_PLATFORM_NAME).should.is_a String
    pf.info(CL_PLATFORM_NAME).should.not.be.empty
    pf.info(CL_PLATFORM_VERSION).should.is_a String
    pf.info(CL_PLATFORM_VERSION).should.not.be.empty
    pf.info(CL_PLATFORM_VENDOR).should.is_a String
    pf.info(CL_PLATFORM_EXTENSIONS).should.is_a String
  end

  the 'invalid query should raise an error.' do
    should.raise(TypeError) { platforms.first.info('profile')}
  end
end
