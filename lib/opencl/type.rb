# encoding: utf-8

module OpenCL
  # A Type object stores the information of a data type, and knows how to
  # represent the type in C99 format.
  #
  # The Type object is used when instantiating OpenCL code templates.
  # You need not to use this class directly.
  class Type
    # Create a type object.
    #
    # (Symbol | Array) tag - The type tag. It is a pre-defined Symbol (in
    #                        OpenCL::SCALAR_TYPES and OpenCL::VECTOR_TYPES),
    #                        or an Array that describes a structured type.
    #
    # Examples
    #
    #   Type.new :cl_float2
    #   Type.new ['code', :cl_uchar, 5, 'age', :cl_uchar, 'revenue', :cl_float]
    def initialize(tag)
      unless Array === tag
        raise ArgumentError, "Unrecognized type: #{tag.to_s}." unless OpenCL.valid_type?(tag)
        @tag = tag
        @size = OpenCL.type_size(tag)
      else
        @tag, @size = parse_structure_tag(tag.clone)
      end
    end

    # Type tag.
    attr_reader :tag
    # Byte size of the data type
    attr_reader :size

    def to_s
      @tag.to_s
    end

    def inspect
      "#<#{self.class}:#{self.object_id} @tag=#{self.tag}>"
    end

    def ==(other)
      self.tag == other.tag
    end

    # Returns a String contains the type definition in C99 format.
    def to_cdef
      @c_def ||= self.number? ? c_def_built_in : c_def_structure
    end

    CLASSIFIERS = [:any, :structure, :number, :scalar, :vector, :exact,
      :integer, :inexact, :float]

    def self.classifier?(symbol)
      CLASSIFIERS.include? symbol
    end

    # Returns true if the receiver is compatible with the given Type or type
    # classifier.
    def compatible?(obj)
      return (self == obj) if obj.is_a?(Type)

      case obj
      when :any
        true
      when :structure
        self.structure?
      when :number
        self.number?
      when :scalar
        self.scalar?
      when :vector
        self.vector?
      when :exact
        self.exact?
      when :inexact
        self.inexact?
      when :float
        self.float?
      when :integer
        self.integer?
      else
        false
      end
    end

    # Returns true if the receiver is a structured type.
    def structure?
      !self.number?
    end

    # Returns true if the receiver is a number, i.e., not a structured type.
    def number?
      # all OpenCL built-in types are number
      OpenCL.valid_type?(self.tag)
    end

    def scalar?
      return false if self.structure?
      !self.vector?
    end

    def vector?
      OpenCL.valid_vector?(self.tag)
    end

    def exact?
      return false if self.structure?
      !self.inexact?
    end
    alias :integer? :exact?

    def inexact?
      return false if self.structure?
      self.tag.to_s =~ /^cl\_(float|half|double)/
    end
    alias :float? :inexact?

    def float?
      self.scalar? && self.inexact?
    end

    def integer?
      self.scalar? && self.exact?
    end

    private

    # Type of field can only be built-in types.
    def parse_structure_tag(tag)
      t = []
      sz = 0
      until tag.empty?
        field_name, type, size = tag
        raise ArgumentError, 'Field name must be a String.' unless field_name.is_a?(String)
        raise ArgumentError, 'Invalid field name.' unless valid_c_name?(field_name)
        raise ArgumentError, 'Invalid type tag.' unless type.is_a?(Symbol)
        type = Type.new(type)
        if size.nil? || size.is_a?(String)
          size = 1
          tag.shift 2
        elsif size.is_a?(Fixnum) && size > 0
          tag.shift 3
        else
          raise ArgumentError, 'Invalid field size.'
        end
        t += [field_name, type, size]
        sz += type.size * size
      end
      [t, sz]
    end

    # Roughly check if the field name of a Structure data type is a valid
    # identifier in C. Might be wrong if the name is a C keyword.
    def valid_c_name?(name)
      return false if name.empty?
      name =~ /^[a-zA-Z_][a-zA-Z0-9_]*$/
    end

    def c_def_built_in
      tmpl = <<-EOT
typedef #{c_name(self)} T;
#define LOAD
#define STORE
      EOT
      ERB.new(tmpl).result(binding)
    end

    def c_name(type)
      raise ArgumentError, 'Expected a built-in data type.' unless type.number?
      type.tag.to_s[3..-1]
    end

    def c_def_structure
      t = self.tag.dup
      tmpl = <<-EOT
typedef struct {
  <% until t.empty? do %>
    <% name, type, size = t.shift(3) %>
    <% if size == 1 %>
      <%= c_name(type) %> <%= name %>;
    <% else %>
      <%= c_name(type) %> <%= name %>[<%= size %>];
    <% end %>
  <% end %>
} T;
#define LOAD
#define STORE
      EOT
      ERB.new(tmpl, nil, '>').result(binding)
    end
  end
end
