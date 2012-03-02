# encoding: utf-8

module OpenCL
  # Image class wrapps the OpenCL image memory object.
  #
  class Image

    # Supported options:
    #   :data
    #   :context
    #   :format
    #
    # Example:
    #   Image.new [100, 100]
    #   Image.new [100, 200], :data = pixel_buffer
    def initialize(size, options = {})
      @width, @height, @depth = size.map(&:to_i)
      @height ||= 1
      @depth ||= 1

      unless @width > 0 && ( (@height > 0 && @depth == 1) || (@height > 1 && @depth > 1))
        raise ArgumentError, 'invalid image size.'
      end
      @size = [@width, @height, @depth]
      @dimension = @size.reduce(0) do |dim, num|
        dim += 1 if num > 1
        dim
      end

      @context = options[:context] || OpenCL::Context.default_context
      unless @context.is_a?(Context)
        raise ArgumentError, "expected option :context is a OpenCL::Context, got a (#{@context.class})."
      end

      fmt = options[:format] || [CL_RGBA, CL_UNORM_INT8]
      raise ArgumentError, 'expected an Array.' unless fmt.is_a?(Array)
      @format = Capi::ImageFormat.new *fmt

      @byte_size = @format.element_size * @width
      @byte_size *= @height if @height > 1
      @byte_size *= @depth if self.threed?

      # TODO: support create Iamge from GL objects.
      data = options[:data]
      unless data.nil?
        raise ArgumentError, "expected an OpenCL::HostPointer, got a (#{data.class})." unless data.is_a?(HostPointer)
        raise ArgumentError, "size of option :data doesn't match the specified image size and format." if data.byte_size != @byte_size
      end

      io = CL_MEM_READ_WRITE
      @memory = if self.threed?
        @context.create_image_3d(io, @format, @width, @height, @depth, 0, 0, data)
      else
        @context.create_image_2d(io, @format, @width, @height, 0, data)
      end
    end

    def initialize_copy(img)
      super

      io = CL_MEM_READ_WRITE
      if self.threed?
        @memory = @context.create_image_3d(io, @format, @width, @height, @depth, 0, 0, nil)
      else
        @memory = @context.create_image_2d(io, @format, @width, @height, 0, nil)
      end
      # copy full image.
      origin = [0, 0, 0]
      region = [self.width, self.height, self.depth]
      cq = @context.command_queue_of @context.default_device
      cq.enqueue_copy_image(img.memory, @memory, origin, origin, region, nil)
    end

    def inspect
      "#<#{self.class} #{self.width}x#{self.height}x#{self.depth} #{self.format}>"
    end
    # Capi::Memory object.
    attr_reader :memory

    attr_reader :byte_size

    attr_reader :dimension

    attr_reader :width

    attr_reader :height

    attr_reader :depth

    def format
      @format.to_a
    end

    def oned?
      @dimension == 1
    end

    def twod?
      @dimension == 2
    end

    def threed?
      @dimension == 3
    end

    # Changes the size of the receiver.
    #
    # Returns the receiver.
    def resize(new_size)
      w, h, d = new_size.map(&:to_i)
      # TODO:
      return self
    end

    # Returns a Image object.
    def copy(origin = [0, 0, 0], size = nil)
      sz = size || @size
      # TODO:
      return nil
    end

    # Writes contents of host memory to the receiver.
    #
    # Returns the receiver.
    def get_data_from(ptr)
      rw_image ptr, :write
    end
    alias :write :get_data_from

    # Reads contents of the receiver to host memory.
    #
    # Returns the receiver.
    def store_data_to(ptr)
      rw_image ptr, :read
    end
    alias :read :store_data_to

    private

    def rw_image(ptr, io)
      raise ArgumentError, "expected a OpenCL::HostPointer, got a (#{ptr.class})." unless ptr.is_a?(HostPointer)
      raise ArgumentError, "" if ptr.byte_size != self.byte_size

      cq = @context.command_queue_of @context.default_device
      if io == :read
        cq.enqueue_read_image(self.memory, true, [0, 0, 0], @size, 0, 0, ptr, nil);
      else
        cq.enqueue_write_image(self.memory, true, [0, 0, 0], @size, 0, 0, ptr, nil);
      end
      self
    end
  end
end
