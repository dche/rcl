# encoding: utf-8

require File.join(File.dirname(__FILE__), '../compile_spec_helper')

ExprShouldPass = [
  '1',
  'a',
  'next',
  'break 0',
  'a = 1',
  '(abc)',
  'b = (abc)',
  'begin a end',
  'begin begin begin a end end end',
  'CLMath.sin(x)',
  'CLMath.cos(x);',
  'c = CLMath.tan(y)',
  'sin(x)',
  'd = sin x',
  'plus a, b, c, 1',
  'plus (a), b, c, 1',
  'plus ((a), b, c, d)',
  'mul a, (b), c, ((d))',
  'plus ((((((a), b)))))',
  'e = plus(a)',
  'e=plus a,b,   c;',
  "1\n2",
  'a;b',
  'x = a;b',
  'a;;;;;b     ',
  'a b; c d;  ',
  'not a',
  'not not not not a',
  'y=not(a)',
  'not BuiltIn.fmod(a)',
  'a and b',
  'a and b or c',
  'x = a and b;',
  'x=(a and b)',
  'a if b',
  'a if b and c',
  'redo if true',
  'a unless b if a until c while d',
  "while a\n b end",
  "u = while a\n b end",
  "while(a)\n c end",
  "while a and b\n c end",
  "while(a or b)\n c end",
  "until a do\n b end",
  "until(a and b) do; c; end;",
  "if a then\n b end",
  "if (a)\n b end;",
  "x = if (a); b end;",
  "z=unless(a) then\n;b;end",
  "sin(x)\n;;\nif a; b; end; cos(y)",
  "unless b then   \n  c end",
  "if a\n b else\n c\n d end",
  "unless a and b then; c \n else; d end",
  "if (a)\n b ;elsif c\n d\n end",
  "if(a nd b) then; c; elsif d then; e; elsif f then g; else; h; end;",
  "case a\nwhen b\n c\n when d\n d\n else\n e\n end",
  "case(a);when b then; c; when(d); e end",
  "true and if true then; false; end or true",
  "a ? b : c",
  "x = a ? b : c",
  "a and b ? c : d",
  "a or b ?c:d",
  "a ? b = c : d = e",
  '1+1',
  'a + 2 * 3',
  '- a ** b',
  'x = y + z',
  'sin(x) * cos(y)',
  'a = x[123]',
  "a = x [12\n]",
  "a [x] = y",
  "a [x\n] = y[0765]",
  "a[y] = true",
  "a.hi",
  "a.xxx",
  'a.xYZZWW',
  "a.s123Abc = b.zz",
  'a.SAABBCC = 1, 2, 3, 4',
  'x =(a, b, c, d)',
  ]

ExprShouldFail = [
  'a,',
  'y=not a',
  '1 = 1',
  'obj(x) = cos(x)',
  '! not true',
  'not if true; a; else; b; end',
  'return a and b',
  "a ? b and c : d",
  "a or b?c:d",
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
      if pr.nil?
        puts '-' * 20
        puts str
        puts '-' * 20
        puts parser.failure_reason
        puts '-' * 20
      end
      pr.should.not.be.nil
    end
  end

  it 'should fail' do
    ExprShouldFail.each do |str|
      parser.parse(str).should.be.nil
    end
  end
end
