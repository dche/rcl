# encoding: utf-8

module OpenCL
  # A Library object stores the source code of OpenCL kernels, and ruby side
  # methods for execute kernels conveniently.
  #
  # To use a library, an Operand class calls the Operand::use() in its
  # class definition.
  #
  class Library

    class <<self
      # The OpenCL kernel source.
      attr_reader :source
      # A Module that defines methods to call kernels defined in the receiver.
      attr_reader :interface_module

      # Sets the type of Operand objects that the Library intends to serve.
      #
      # type_classifier can be a Type object, a type tag, or a Symbol that
      # represents a class of types, e.g., :all for all types, :number for
      # built-in OpenCL types.
      def type(type_classifier = nil)
        return @type if type_classifier.nil?
        raise RuntimeError, "Can't change type of a Library." if @type

        if type_classifier.is_a?(Type) || Type.classifier?(type_classifier)
          @type = type_classifier
        else
          @type = Type.new type_classifier
        end
      end

      # Defines an kernel by providing kernel name and source code.
      def def_kernel(kernel_name, &blk)
        @kernels ||= []

        if @kernels.include?(kernel_name)
          warn "Knerel with duplicated name exists. Operation ignored."
        else
          @kernels << kernel_name
          @source ||= ''
          @source << yield
        end
      end

      # Defines a method that executes OpenCL kernels.
      def def_method(meth, &blk)
        @interface_module ||= Module.new
        @interface_module.class_eval do
          define_method(meth, &blk)
        end
      end

      def alias_method(new_name, old_name)
        @interface_module ||= Module.new
        @interface_module.class_eval do
          alias_method new_name, old_name
        end
      end
    end
  end
end
