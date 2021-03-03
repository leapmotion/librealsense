#  Minimum required CMake version: 3.1.0
cmake_minimum_required(VERSION 3.1.0)
project(librealuvc LANGUAGES CXX C)

# -------
# Modules
# -------

# Include modules in "{project}/CMake/".
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" CACHE PATH "cmake modules" FORCE)

# ---------------------
# General Configuration
# ---------------------

include(CMake/global_config.cmake)

# C++ flags.
config_cxx_flags()

# Include OS-specific macros.
# Macro definitions located at "CMake/global_config.cmake".
include(CMake/include_os.cmake)

# Set OS-specific configuration flags.
# Macro definitions located at "CMake/<OS>_config.cmake".
os_set_flags()

# Set global configuration flags.
# Macro definitions located at "CMake/global_config.cmake".
global_set_flags()

# ---------
# Libraries
# ---------

message(STATUS "add_library: ${LRS_TARGET} SHARED: ${BUILD_SHARED_LIBS}")
add_library(${LRS_TARGET} "")

# ---------------------
# Libraries - via Conan
# ---------------------

# Basic Conan setup. conanbuildinfo.cmake is produced by the `conan install` command, which references conanfile.py.
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# To support OpenCV and any other conan-based libraries, conan_basic_setup() is almost all we need. However, the test targets require specific add_library invocations, which is provided by this additional macro provided by conanbuildinfo.cmake.
# 
# This invokes add_library(CONAN_PKG::opencv ...), which replaces old references to the OpenCV dependency from before the conan setup. In src/CMakeLists.txt executables that use OpenCV for viewing camera output are now able to depend on CONAN_PKG::opencv and any other specific packages we're getting through conan.
conan_define_targets()

# -------------------
# OpenCV -- Reference
# -------------------
# Old non-Conan OpenCV inclusion.

# # add_OpenCV() (from global_config.cmake)
# find_package(OpenCV REQUIRED)
# set(DEPENDENCIES librealuvc ${OpenCV_LIBS})
# include_directories(${OpenCV_INCLUDE_DIRS})
# add_library(OpenCV SHARED IMPORTED)

# ------------------
# More Configuration
# ------------------

# Global project configuration.
# Macro definitions located at "CMake/global_config.cmake".
global_target_config()

# Configure the project according to OS-specific requirments.
# Macro definitions located at "CMake/<OS>_config.cmake".
os_target_config()

# ----------
# CMakeLists
# ----------

include(include/CMakeLists.txt)
include(src/CMakeLists.txt)
include(third-party/CMakeLists.txt)

# -------------
# Build Version
# -------------

set_target_properties(${LRS_TARGET} PROPERTIES
  WINDOWS_EXPORT_ALL_SYMBOLS ON
  VERSION ${REALUVC_VERSION_STRING}
  SOVERSION ${REALUVC_VERSION_MAJOR}
)

# -----------------------
# Included Subdirectories
# -----------------------

add_subdirectory(wrappers)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(BUILD_UNIT_TESTS)
  add_subdirectory(unit-tests)
endif()

# --------------------------------
# Installation Build Target Config
# --------------------------------

include(CMake/install_config.cmake)
