
add_executable(c_example c_example.c elfio_c_wrapper.cpp elfio_c_wrapper.h)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR})
