
require 'mkmf'

$CFLAGS += ' -std=c99 -fvisibility=hidden'

if RUBY_ENGINE == 'macruby'
  $CFLAGS += ' -DHAVE_MACRUBY=1'
end

case RUBY_PLATFORM
when /darwin/
  $LDFLAGS += ' -framework OpenCL'
when /linux/
  $LDFLAGS += ' -lOpenCL'
else
  raise RuntimeError, "ruby platform #{RUBY_PLATFORM} is not supported."
end

create_makefile('opencl/capi')
