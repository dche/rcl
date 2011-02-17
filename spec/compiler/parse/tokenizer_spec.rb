# encoding: utf-8

require File.join(File.dirname(__FILE__), '../compile_spec_helper')

bc = <<-EOC

=begin
Hello, this is a block comments.
We must put a new line befor =begin. FIXME?
  =end
    =end
=begin
=end
  EOC

SeparatorShouldPass = [
    "#abcd\r\n",
    "   # safa ## abc abc\n",
    "\n\r     \t \n \t  # comments.\n    ",
    bc,
    ' ' * 20,
    "\t   \t   \t",
    ";\n;; ;   \n;",
    "\n\n\n\n",
    "",
    ';'
  ]

parser = RubyTokenizerParser.new

describe RubyTokenizerParser do
  it 'should pass' do
    SeparatorShouldPass.each do |str|
      parser.parse(str).should.not.be.nil
    end
  end
end
