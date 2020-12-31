from conans import ConanFile, CMake

class PocoTimerConan(ConanFile):
  settings = "os", "compiler", "build_type", "arch"
  # requires=[
  #   "boost/1.70.0@conan/stable",
  #   "eigen/3.3.7@conan/stable",
  #   "opencv/4.1.0@conan/stable",
  #   "protobuf/3.8.0@conan/stable"
  # ]
  requires=[
    "boost/1.70.0",
    "eigen/3.3.7",
    "opencv/4.5.0",
    "protobuf/3.9.1"
  ]
  generators = "cmake"
  
  # Configuration for dependencies.
  # default_options = {"poco:shared": True, "openssl:shared": True}

  # e.g. copy all dll files from the "bin" package folder to the "bin" project folder
  # e.g. copy all dylib files from the "lib" package folder to the "bin" project folder 
  #  def imports(self):
  #     self.copy("*.dll", dst="bin", src="bin") # From bin to bin
  #     self.copy("*.dylib*", dst="bin", src="lib") # From lib to bin
  
  def build(self):
      cmake = CMake(self)
      cmake.configure()
      cmake.build()
