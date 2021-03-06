
require 'rake'
require 'rubygems/package_task'

require 'rbconfig'

spec = Gem::Specification.new do |s|
  s.name = 'rcl'
  s.version = '0.3.3'
  s.summary = 'Exploiting the power of GPUs.'
  s.description = s.summary

  s.author = 'Che Kenan'
  s.email = 'chekenan@gmail.com'

  s.files = %w(README.md LICENSE Rakefile) +
            FileList['{lib,spec,bm}/**/*.rb'] +
            FileList['ext/*.{rb,c,h}']
  s.extensions << 'ext/extconf.rb'

  s.required_ruby_version = '>= 1.9'
  s.platform = Gem::Platform::RUBY

  s.add_development_dependency 'bacon', '>= 1.1.0'
  s.add_development_dependency 'simplecov', '>= 0.5.0'
  s.add_development_dependency 'narray', '>= 0.5.9'
  s.add_development_dependency 'perftools.rb', '>= 0.5.6'

  s.rdoc_options << '--title' << 'Ruby OpenCL' <<
                    '--main' << 'README.md' <<
                    '--line-numbers' << '--exclude="spec/*"'
end

Gem::PackageTask.new(spec) do |p|
  p.need_zip = true
  p.need_tar = true
end

dir = File.dirname(__FILE__)
extdir = File.join(dir, 'ext')
libdir = File.join(dir, 'lib/opencl')
# special case for JRuby, whose 'DLEXT' is 'jar'.
ext_ext = RbConfig::CONFIG['DLEXT']
if RUBY_ENGINE == 'jruby'
  ext_ext = case RbConfig::CONFIG['target_os']
  when 'darwin'
    'bundle'
  when 'linux'
    'so'
  else
    raise "target os '#{RbConfig::CONFIG['target_os']}' is not supported."
  end
end

ext_target = "capi.#{ext_ext}"

task :default => :spec

desc 'Run specification using Bacon.'
task :spec => :build do
  begin
    require 'bacon'
    require File.join(dir, 'spec/spec_helper')

    Bacon.extend Bacon::TapOutput
    FileList[File.join(dir, 'spec/**/*_spec.rb')].each do |f|
      load f
    end

    Bacon.handle_summary
  rescue LoadError
    $sederr.puts 'bacon is needed to run specification.'
  end
end

desc 'Install locally.'
task :install => :gem do
  system "gem install pkg/#{spec.name}-#{spec.version}.gem"
end

file File.join(libdir, ext_target) => FileList[File.join(extdir, '*.{c,h}')] do
  cd(extdir, verbose:false) do
    system "#{RbConfig::CONFIG['bindir']}/#{RUBY_ENGINE} extconf.rb && make"
  end
  cp File.join(extdir, ext_target), libdir, verbose:false
end

desc 'Build the extension.'
task :build => File.join(libdir, ext_target)

desc 'Clean the project directory.'
task :clean => :clobber_package do
  FileList['**/*.{rbc,rbo,o,bundle,so,dll}'].sort.each do |f|
    rm f
  end
end

desc 'Rebuild the extension.'
task :rebuild => [:clean, :build]
