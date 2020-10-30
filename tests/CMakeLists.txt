# Unit tests are written using the Boost unit test framework
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

# Find all the binary files used for testing and copy them into the build
# directory. This allows the test to be run from the build directory

# First, create an elf_examples folder under the current build directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/elf_examples)

# Second, glob all files under elf_examples
file(GLOB elf_examples
    LIST_DIRECTORIES
        false
    CONFIGURE_DEPENDS
    elf_examples/*)

# Third, copy each file globbed to the elf_examples folder under the current
# build directory
foreach(example ${elf_examples})
    configure_file(${example} ${CMAKE_CURRENT_BINARY_DIR}/elf_examples COPYONLY)
endforeach()

# Lastly, copy the script to run the tests
configure_file(runELFtests ${CMAKE_CURRENT_BINARY_DIR}/runELFtests COPYONLY)

add_executable(
    ELFIOTest
    ELFIOTest.cpp
    ELFIOTest1.cpp
    ELFIOTest2.cpp)

target_link_libraries(
    ELFIOTest
    PRIVATE
        elfio::elfio
        Boost::unit_test_framework)

add_test(
    NAME
        ELFIOTest
    COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/ELFIOTest
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR})

add_dependencies(check ELFIOTest)
