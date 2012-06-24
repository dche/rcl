#
# This file expands the CAPI classes and methods defined in C extension,
# to make CAPI more easy to use. However, even though,
# CAPI still is *NOT* expected to be used by the library users.
# You should use more rubyish APIs that defiend in module OpenCL, e.g.,
# Context, Program, Buffer, Operand and NDArray
#

# The OpenCL enabler for ruby.
module OpenCL
  module Capi
    class Platform

      def profile
        self.info(CL_PLATFORM_PROFILE)
      end

      def full_profile?
        self.profile == 'FULL_PROFILE'
      end

      def embedded_profile?
        self.profile == 'EMBEDDED_PROFILE'
      end

      # A String contains the name of the receiver.
      def name
        self.info(CL_PLATFORM_NAME)
      end

      # Returns the vendor name of the receiver.
      def vendor
        self.info(CL_PLATFORM_VENDOR)
      end

      # Returns the version of the receiver.
      #
      # Example:
      #
      #   aPlatform.version # => "1.0"
      def version
        self.info(CL_PLATFORM_VERSION)
      end

      # Returns an Array contains the extension strings.
      def extensions
        self.info(CL_PLATFORM_EXTENSIONS).split(' ')
      end

      def to_s
        "#<#{self.class} : name = #{self.name} : version = #{self.version}>"
      end
    end

    class Device

      # Returns +true+ if device is available.
      def available?
        self.info(CL_DEVICE_AVAILABLE)
      end

      # Retruns +true+ if device type is GPU.
      def gpu?
        (self.info(CL_DEVICE_TYPE) & CL_DEVICE_TYPE_GPU) == CL_DEVICE_TYPE_GPU
      end

      def support_image?
        self.info(CL_DEVICE_IMAGE_SUPPORT)
      end

      def little_endian?
        self.info(CL_DEVICE_ENDIAN_LITTLE)
      end

      def big_endian?
        not self.little_endian?
      end

      def max_clock_frequency
        self.info(CL_DEVICE_MAX_CLOCK_FREQUENCY)
      end

      # The number of parallel compute cores on the OpenCL device.
      def max_compute_units
        self.info(CL_DEVICE_MAX_COMPUTE_UNITS)
      end

      def max_image2d_size
        return nil unless self.support_image?

        [self.info(CL_DEVICE_IMAGE2D_MAX_WIDTH),
          self.info(CL_DEVICE_IMAGE2D_MAX_HEIGHT)]
      end

      def max_image3d_size
        return nil unless self.support_image?

        [self.info(CL_DEVICE_IMAGE3D_MAX_WIDTH),
          self.info(CL_DEVICE_IMAGE3D_MAX_HEIGHT),
          self.info(CL_DEVICE_IMAGE3D_MAX_DEPTH)]
      end

      # Maximum number of work-items in a work-group executing a kernel
      # using the data parallel execution model.
      def max_workgroup_size
        self.info(CL_DEVICE_MAX_WORK_GROUP_SIZE)
      end

      # Maximum number of work-items that can be specified in each dimension
      # of the work-group to clEnqueueNDRangeKernel.
      #
      # Returns an Array contains the number for each dimension.
      def max_work_item_sizes
        self.info(CL_DEVICE_MAX_WORK_ITEM_SIZES)
      end

      def base_address_align
        self.info(CL_DEVICE_MEM_BASE_ADDR_ALIGN)
      end

      def address_bits
        self.info(CL_DEVICE_ADDRESS_BITS)
      end

      def max_mem_alloc_size
        self.info(CL_DEVICE_MAX_MEM_ALLOC_SIZE)
      end

      def global_memory_size
        self.info(CL_DEVICE_GLOBAL_MEM_SIZE)
      end

      def local_memory_size
        @lms ||= self.info(CL_DEVICE_LOCAL_MEM_SIZE)
      end

      def vendor
        self.info(CL_DEVICE_VENDOR)
      end

      def name
        self.info(CL_DEVICE_NAME)
      end

      def cl_version
        self.info(CL_DEVICE_VERSION)
      end

      def profile
        self.info(CL_DEVICE_PROFILE)
      end
      alias :version :cl_version

      def driver_version
        self.info(CL_DRIVER_VERSION)
      end

      def to_s
        "#<#{self.name} #{self.vendor} #{self.cl_version} #{self.driver_version}>"
      end
    end

    class Context
      # Returns an Array of devices attached in the receiver.
      #
      # Same effect as +Context#info(CL_CONTEXT_DEVICES)+.
      def devices
        @devices ||= self.info(CL_CONTEXT_DEVICES).freeze
      end

      def create_command_queue(dev = nil)
        dev ||= self.devices.first
        CommandQueue.new(self, dev, 0)
      end

      def create_profiling_command_queue(dev = nil)
        dev ||= self.devices.first
        CommandQueue.new(self, dev, CL_QUEUE_PROFILING_ENABLE)
      end

      def create_program(src)
        Program.new(self, [src])
      end

      def create_memory_object(type, *args)
        Memory.new(type, self, *args)
      end

      def create_buffer(flags, size, host_ptr = nil)
        Memory.new(:buffer, self, flags, size, host_ptr)
      end

      def create_image_2d(*args)
        Memory.new(:image_2d, self, *args)
      end

      def create_image_3d(*args)
        Memory.new(:image_3d, self, *args)
      end
    end

    class Program
      def create_kernel(name)
        Kernel.new(self, name)
      end
    end

    # The OpenCL kernel object.
    #
    # A kernel object encapsulates the specific __kernel
    # function declared in a program and the argument values to be used
    # when executing this __kernel function.
    #
    class Kernel
      # A String contains the kernel name.
      def function_name
        @name ||= self.info(CL_KERNEL_FUNCTION_NAME).freeze
      end
      alias :name :function_name

      # Number of kernel arguments.
      def argument_number
        @num_args ||= self.info(CL_KERNEL_NUM_ARGS)
      end

      # The maximum work-group size that can be used to execute a kernel
      # on a specific device given by device.
      #
      # The OpenCL implementation uses the resource requirements of
      # the kernel (register usage etc.) to determine
      # what this workgroup size should be.
      def workgroup_size_on_device(dev)
        self.workgroup_info(dev, CL_KERNEL_WORK_GROUP_SIZE)
      end

      # Returns the amount of local memory in bytes being used by a kernel.
      #
      # This includes local memory that maybe needed by an implementation
      # to execute the kernel, variables declared inside the kernel
      # with the __local address qualifier and local memory to be allocated
      # for arguments to the kernel declared as pointers
      # with the __local address qualifier and whose size is specified
      # with +set_arg()+.
      #
      def local_memory_size
        devs = self.info(CL_KERNEL_CONTEXT).devices
        self.workgroup_info(devs.length == 1 ? nil : devs.first, CL_KERNEL_LOCAL_MEM_SIZE)
      end
    end

    class Memory
      def self.new(type, *args)
        case type
        when :buffer
          self.create_buffer(*args)
        when :image_2d
          self.create_image_2d(*args)
        when :image_3d
          self.create_image_3d(*args)
        else
          raise ArgumentError, "invalid memory object type."
        end
      end
    end

    class ImageFormat
      def to_s
        self.to_a.to_s
        # TODO: should show the names of constants, instead of numbers.
      end

      def to_a
        [self.channel_order, self.channel_data_type]
      end
    end
  end

  # :nodoc:
  module PointerHelper

    # Assign the values of an Array to the receiver.
    #
    # Example::
    #
    #  ptr = HostPointer.new :cl_uint, 3
    #  ptr.assign [1, 2, 3]
    #  ptr.assign [1], 2      # ptr[2] = 1
    def assign(ary, offset = 0)
      case ary
      when Array
        offset.upto(self.size - 1) do |i|
          ai = i - offset
          break if ai >= ary.size

          self[i] = ary[ai]
        end
      when String
        self.assign_byte_string ary, offset
      else
        raise ArgumentError, "expected Array or String, but got #{ary.class}."
      end
      return self
    end

    def type_size
      OpenCL.type_size self.type
    end

    # Returns +true+ if the Pointer is null.
    def null?
      self.address.nil?
    end

    # Two pointer is equal if they have same address.
    def ==(p)
      p.address == self.address
    end

    def inspect
      "#<#{self.class}: address=#{self.address}, type=#{self.type}, size=#{self.size}>"
    end

    def to_a
      ary = []
      self.size.times do |i|
        ary.push self[i]
      end
      ary
    end

    def to_i
      "0x%016x" % self.address
    end
  end

  # A piece of managed memeory region.
  #
  # +HostPointer+ represents the host memory region.
  # To transfor data to/from devices, you must use +HostPointer+
  # to manage (allocate, free and access) the host memory.
  class HostPointer
    include PointerHelper

    def copy_to(p)
      p.copy_from(self)
      self
    end
  end

  class MappedPointer
    include PointerHelper
  end
end
