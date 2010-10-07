

class String

  # Include a common OpenCL utility module. The effect of this method is
  # read the contents of a OpenCL source file from a predefined
  # position (at present, it's the 'cl' directory under 'lib'), and then
  # append the contents to the receiver.
  #
  # For example, you want to use a random generator in your kernel, so
  # you reference the RNG methods in your kernel source code.
  # Before you build the program with source, you need
  # call your kernel string variable's +rcl_include+ method, with the
  # arguemnt +'random'+, which is the file name that defines the RNG methods.
  #
  def rcl_include(mod_name)
    fname = "#{mod_name}.cl"
    path = File.join(File.dirname(__FILE__), '../cl', fname)
    unless File.file?(path)
      raise ArgumentError, "No OpenCL source named '#{mod_name}.cl' found."
    end

    self << File.open(path) do |f|
      f.read
    end
  end
end
