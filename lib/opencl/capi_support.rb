
#
# This file expands the CAPI classes and methods, to make CAPI more easy
# to use. However, even this, CAPI still is *NOT* expected to be used
# by the library user. You should use more rubyish API that defiend in
# opencl.rb.
#                    
# copyright (c) 2010, Che Kenan

require File.join(File.dirname(__FILE__), 'capi')

module OpenCL
  module Capi
    # OpenCL version the API conforms to.
    VERSION = '1.0'

    class Platform
      # Returns the string name of the receiver.
      def name
        self.info(CL_PLATFORM_NAME)
      end

      # Returns the string vendor name of the receiver.
      def vendor
        self.info(CL_PLATFORM_VENDOR)
      end

      # Returns the string version of the receiver.
      #
      # Example:
      #
      #   aPlatform.version # => "Apple"
      def version
        self.info(CL_PLATFORM_VERSION)
      end

      def to_s
        "#{self.vendor} #{self.name} #{self.version}"
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

      def max_workgroup_size
        self.info(CL_DEVICE_MAX_WORK_GROUP_SIZE)
      end

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
        self.info(CL_DEVICE_LOCAL_MEM_SIZE)
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
        "#{self.name} #{self.vendor} #{self.cl_version} #{self.driver_version}"
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
    class Kernel
      def function_name
        @name ||= self.info(CL_KERNEL_FUNCTION_NAME).freeze
      end
      alias :name :function_name

      def argument_number
        @num_args ||= self.info(CL_KERNEL_NUM_ARGS)
      end

      def workgroup_size_on_device(dev)
        self.workgroup_info(dev, CL_KERNEL_WORK_GROUP_SIZE)
      end
      
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
          raise ArgumentError, "Invalid memory object type."
        end
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
        raise ArgumentError, "Expected Array or String, but got #{ary.class}."
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

    # Always +false+.
    def eql?(p)
      false
    end

    def inspect
      "#<#{self.class} address=#{self.address} type=#{self.type} size=#{self.size}>"
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
