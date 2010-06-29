# encoding: utf-8
#
# This file expands the CAPI classes and methods, to make CAPI more easy
# to use. However, even this, CAPI still is *NOT* expected to be used
# by the library user. You should use more rubyish API that defiend in
# opencl.rb.
#                    
# copyright (c) 2010, Diego Che

require File.join(File.dirname(__FILE__), 'capi')

module OpenCL
  module Capi
    
    # Common methods for all classes that represents an OpenCL objects.
    module CL_Object
      
    end
    
    class Platform
    end
    
    class Device
    end
    
    class Context
      include CL_Object
      
      # Returns an Array of devices attached in the receiver.
      #
      # Same effect as +Context#info(CL_CONTEXT_DEVICES)+.
      def devices
        @devices ||= self.info(CL_CONTEXT_DEVICES)
      end
      
      def create_command_queue(dev = nil)
        dev ||= devices.first
        CommandQueue.new(self, dev, 0)
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
      include CL_Object
      
      def create_kernel(name)
        Kernel.new(self, name)
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
        self.address
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
end
