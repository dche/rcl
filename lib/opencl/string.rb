

class String
  
  # Include a common OpenCL utility module.
  #
  # For example, you want to use random generator in your kernel.
  # Before you build the program with source, you need
  # call your kernel string variable's +rcl_include+ method, with the
  # arguemnt +'random'+.
  #
  # To get the names of all available utility modules, 
  # call +OpenCL.utility_modules+.
  def rcl_include(mod_name)
    fname = "#{mod_name}.cl"
    path = File.join(File.dirname(__FILE__), 'cl', fname)
    unless File.file?(path)
      raise ArgumentError, "No OpenCL source named '#{mod_name}.cl' found."
    end
    
    self << File.open(path) do |f|
      f.read
    end
  end
end
