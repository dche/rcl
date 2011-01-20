# encoding: utf-8

require 'treetop'

Treetop.load File.join(File.dirname(__FILE__), 'tokenizer')
Treetop.load File.join(File.dirname(__FILE__), 'number')
Treetop.load File.join(File.dirname(__FILE__), 'identifier')
Treetop.load File.join(File.dirname(__FILE__), 'expression')
Treetop.load File.join(File.dirname(__FILE__), 'block')
