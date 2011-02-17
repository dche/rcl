# encoding: utf-8

require File.join(File.dirname(__FILE__), '../compile_spec_helper')

parser = RubyBlockParser.new

BlockShouldPass = [
  '|x, y,z, u,  v, www | x;y;u;v;www',
  '|a|; 1',
  "|  x |\n;;x[0];",
  '| x   |   x + y',
  '|x|x',
  ]

BlockShouldFail = [
  '|,|',
  ]


describe RubyBlockParser do
  it 'should pass' do
    BlockShouldPass.each do |str|
      parser.parse(str).should.not.be.nil
    end
  end

  it 'should fail' do
    BlockShouldFail.each do |str|
      parser.parse(str).should.be.nil
    end
  end
end
