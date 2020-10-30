# Install script for directory: F:/Projects/elfio/examples

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "F:/Projects/elfio/out/install/x64-Debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("F:/Projects/elfio/out/build/x64-Debug/examples/add_section/cmake_install.cmake")
  include("F:/Projects/elfio/out/build/x64-Debug/examples/anonymizer/cmake_install.cmake")
  include("F:/Projects/elfio/out/build/x64-Debug/examples/elfdump/cmake_install.cmake")
  include("F:/Projects/elfio/out/build/x64-Debug/examples/tutorial/cmake_install.cmake")
  include("F:/Projects/elfio/out/build/x64-Debug/examples/write_obj/cmake_install.cmake")
  include("F:/Projects/elfio/out/build/x64-Debug/examples/writer/cmake_install.cmake")
  include("F:/Projects/elfio/out/build/x64-Debug/examples/c_wrapper/cmake_install.cmake")

endif()

