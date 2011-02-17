
require 'mkmf'

$CFLAGS += ' -ansi -std=c99'

case RUBY_PLATFORM
when /darwin/
  $LDFLAGS += ' -framework OpenCL'
when /linux/
  $LDFLAGS += ' -lOpenCL'
else
  raise RuntimeError, "platform #{RUBY_PLATFORM} is not supported."
end

create_makefile('opencl/capi')

