cmake_minimum_required(VERSION 3.12.4)

project(ELFIOTest)
set(CMAKE_CXX_STANDARD 11)

add_executable(ELFIOTest ELFIOTest.cpp ELFIOTest1.cpp ELFIOTest2.cpp)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/..)
