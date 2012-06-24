
module OpenCL
  module Capi
    class Context
      def number_of_devices
        self.info CL_CONTEXT_NUM_DEVICES
      end
    end
  end
end
