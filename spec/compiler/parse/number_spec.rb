# encoding: utf-8

require File.join(File.dirname(__FILE__), '../compile_spec_helper')

NumberShouldPass = [
    '12345',
    '-12345',
    '1_2_3_45',
    '0765',
    '-0',
    '-071',
    '0xabc',
    '-0xA_B_C_def1123',
    '0b0111',
    '0B00_110_1111_000',
    '-1.0',
    '1_2_3.0000_4_5_6',
    '1.0e-10_0',
    Math::PI.to_s,
    '0.1234e567',
    '0.1',
    '000',
    '0d0',
    '-0D0',
    '0d12390',
    '0o12',
    '-0_765',
  ]

NumberShouldFail = [
    '1_',
    '0789',
    '0b1002',
    '00.12',
    '0.12e',
    '0o789',
  ]

parser = RubyNumberParser.new

describe RubyNumberParser do
  it 'should pass' do
    NumberShouldPass.each do |str|
      num = parser.parse str
      num.should.not.be.nil
    end
  end

  it 'should fail' do
    NumberShouldFail.each do |str|
      parser.parse(str).should.be.nil
    end
  end
end
