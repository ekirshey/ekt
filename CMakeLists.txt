cmake_minimum_required(VERSION 3.20)
project(ekt CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "_CMake")

# The library holding all of the ekt template machinery
add_subdirectory(src)

# The ekt command line executable
add_subdirectory(app)

option(EKT_BUILD_TESTS "Build the ekt unit tests" ON)
if(EKT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
