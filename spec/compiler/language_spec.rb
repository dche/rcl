# encoding: utf-8

require File.join(File.dirname(__FILE__), 'compile_spec_helper')

class MyLang < Language
  translator(:a) do |sexp|
    self.symbols
  end

  inferrer(:a) do |sexp|
  end
  
  translator(:b) do |sexp|
  end
end

describe Language do
  before do
    @lang = MyLang.new Hash.new
  end
  
  it 'should reject wong symbol table' do
    should.raise(ArgumentError) {
      MyLang.new []
    }
  end
  
  it 'should be able to process :a' do
    should.not.raise(Exception) {
      @lang.infer [:a].extend(Sexp)
      @lang.translate [:a, [:b, "c"]].extend(Sexp)
    }
  end
end

