# encoding: utf-8

require File.join(File.dirname(__FILE__), 'compile_spec_helper')

parser = RubyBlockParser.new

describe "to_sexp" do
  the '#head and #rest' do
    sexp = [:a].extend(Sexp)
    sexp.head.should.equal :a
    sexp.rest.should.be.empty
    [:a, [:a, "b"], :c].extend(Sexp).rest.should.equal [[:a, "b"], :c]
  end

  it 'should respond to ' do
    sexp = [:a, [:b, :c]].extend(Sexp)
    sexp.b.should.equal [:b, :c]
  end

  it 'should produce sexp' do
    (BlockShouldPass + ExprShouldPass).each do |exp|
      p exp
      should.not.raise(Exception) {
        sexp = parser.parse(exp).to_sexp
        puts sexp.extend(Sexp).print
      }
    end
  end

  the '#attribute' do
    sexp = [:a].extend(Sexp)
    sexp.should.be.respond_to :attribute
    sexp.attribute(:type).should.be.nil
    sexp.attribute(:type, :int4)
    sexp.attribute(:type).should.equal :int4
  end
end

