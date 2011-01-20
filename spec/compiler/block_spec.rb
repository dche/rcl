# encoding: utf-8

require File.join(File.dirname(__FILE__), 'compile_spec_helper')

parser = RubyBlockParser.new

ShouldPass = [
  '|x, y,z, u,  v, www | 1',
  '||; 1',
  "|   |\n;;1",
  '| x   |   1',
  '|x|1',
  ]

ShouldFail = [
  '|,|',
  ]


describe RubyBlockParser do
  it 'should pass' do
    ShouldPass.each do |str|
      parser.parse(str).should.not.be.nil
    end
  end

  it 'should fail' do
    ShouldFail.each do |str|
      parser.parse(str).should.be.nil
    end
  end
end
