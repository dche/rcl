
require 'rake'
require 'rake/gempackagetask'

require 'fileutils'
include FileUtils

spec = Gem::Specification.new do |s|
  s.name = 'opencl'
  s.version = '0.1'
  s.summary = 'Exploiting the power of GPUs.'
  s.description = s.summary
  
  s.author = 'Diego Che'
  s.email = 'chekenan@gmail.com'
  
  s.files = %w(README.md LICENSE Rakefile) + 
            FileList['{lib,spec,bm}/**/*.rb'] +
            FileList['ext/*.{rb,c}']
  s.extensions << 'ext/extconf.rb'
  
  s.required_ruby_version = '>= 1.8'
  s.platform = Gem::Platform::RUBY
  
  s.add_development_dependency 'bacon', '>= 1.1.0'
  
  s.rdoc_options << '--title' << 'Ruby OpenCL' <<
                    '--main' << 'README.md' <<
                    '--line-numbers' << '--exclude="spec/*"'
end

Rake::GemPackageTask.new(spec) do |p|
  p.gem_spec = spec
  p.need_zip = true
  p.need_tar = true
end

dir = File.dirname(__FILE__)
extdir = File.join(dir, 'ext')
libdir = File.join(dir, 'lib/opencl')

task :default => :spec

desc 'Run specification using Bacon.'
task :spec => :build do
  begin
    require 'bacon'
    require File.join(dir, 'spec/spec_helper')
  rescue LoadError
    $sederr.puts 'bacon is needed to run specification.'
  end    
  
  Bacon.extend Bacon::TestUnitOutput
  FileList[File.join(dir, 'spec/**/*_spec.rb')].each do |f|
    load f
  end
  
  Bacon.handle_summary
end

rule File.join(extdir, 'capi.bundle') => File.join(extdir, 'capi.c') do  
  cd(extdir) do
    system 'ruby extconf.rb && make'
  end
end

file File.join(libdir, 'capi.bundle') => File.join(extdir, 'capi.bundle') do
  cp File.join(extdir, 'capi.bundle'), libdir
end

desc 'Build the extension.'
task :build => File.join(libdir, 'capi.bundle')

desc 'Clean the project directory.'
task :clean => :clobber_package do
  cd dir do
    FileList['**/*.{rbc,rbo,o,bundle,so,dll}'].sort.each do |f|
      rm f
    end
  end
end

desc 'Run benchmark.'
task :bm => :build do
  FileList[File.join(dir, 'bm/**/*.rb')].each do |bm|
    load bm
  end
end
