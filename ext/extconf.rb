
require 'mkmf'

$CFLAGS += ' -Wall'
$LDFLAGS += ' -framework OpenCL' if RUBY_PLATFORM =~ /darwin/

create_makefile('opencl/capi')
