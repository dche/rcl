# encoding: utf-8

require File.join(File.dirname(__FILE__), 'compile_spec_helper')

ShouldPass = [
    'abc',
    '@abc',
    '@@BCD',
    '_',
    '___',
    '_ABC',
    'xy123',
    'x_1_2',
    'AbcDef',
    'A_B_C',
    'iff',
    'thenn',
    'whilee',
    '@if',
    '@@until',
    '_while',
    '__retry',
    'redo123',
    'BEGIN1',
    'ENDabc',
    ':abc',
    ':a123',
    'true',
    'false',
    'nil',
  ]

ShouldFail = [
    'if',
    'when',
    'while',
    '123abc',
    '^ab',
    '$gvar',
    'class#comment',
    'def\n',
    'BEGIN',
    'END',
    ':123',
  ]

parser = RubyIdentifierParser.new

describe RubyIdentifierParser do
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
