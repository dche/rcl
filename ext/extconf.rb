
require 'mkmf'

$CFLAGS += ' -ansi -std=c99'
$LDFLAGS += ' -framework OpenCL' if RUBY_PLATFORM =~ /darwin/

create_makefile('opencl/capi')
