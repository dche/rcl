
# Ruby OpenCL

**Exploits the power of GPU**

---

RCL is yet another OpenCL library (I avoid to use the term _binding_
because it implies a tight mapping between the ruby API and C API) for
Ruby.

## Dependency

[bacon](http://rubygems.org/bacon) is used in development for unit testing.
You need not install bacon to use this gem.

[randall](http://rubygems.org/randall) is used in development to generate
data for benchmarking and test. You need not install it either.

## Install

    $ gem install opencl

## Examples

    # defines a program.
    Program.new <<- EOP

    EOP
    # provides data to the program


## Limitations


## Links

* RCL

  [Fork me](http://github.com/dche/rcl) on [github](http://github.com).

  [Bugs & Proposals](http://github.com/dche/rcl/issues/) on github too.

  *github* rocks!

* OpenCL resources

  [OpenCL Homepage](http://khronos.org/registry/cl/)

  [OpenCL Online Manuel](http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/)

  [OpenCL programming guide]() from Apple.

* Other OpenCL wrappers/bindings for Ruby

  [ruby-opencl]()

  [barracuda](http://github.com/lsegal/barracuda)

  Ruben Fonseca's [experiment on OpenCL for MacRuby]()

## License

The MIT license.

Copyright (c) 2010, Che Kenan

