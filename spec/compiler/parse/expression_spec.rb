# encoding: utf-8

require File.join(File.dirname(__FILE__), '../compile_spec_helper')

ExprShouldFail = [
  'a,',
  'y=not a',
  '1 = 1',
  'obj(x) = cos(x)',
  '! not true',
  'not if true; a; else; b; end',
  'return a and b',
  "a ? b and c : d\n1234",
  "1+1\na = 4\na or b?c:d",
  <<-EOS,
a = 1
b = 2
if a > b
  true
else
  puts 'ok' # wrong line!
end
c = sin(x)
d[123] = c
c = a + b
  EOS
  "a = b ?c = d:e = f",
  "a = x[y[z[1]]]",
  'a.xy123w',
  'a.234',
  'a.ssee',
  'x = [a,b,c,d]',
  ]

parser = RubyExpressionParser.new

describe RubyExpressionParser do
  it 'should pass' do
    ExprShouldPass.each do |str|
      pr = parser.parse(str)
      raise ParseError.new(parser) if pr.nil?
      pr.should.not.be.nil
    end
  end

  it 'should fail' do
    ExprShouldFail.each do |str|
      parser.parse(str).should.be.nil
      puts ParseError.new(parser)
    end
  end
end
