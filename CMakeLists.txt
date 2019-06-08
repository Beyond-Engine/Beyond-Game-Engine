cmake_minimum_required(VERSION 3.8)

project(BeyondGameEngine VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include("CMakeDependentOption")

option(BEYOND_BUILD_TESTS "Builds the tests" OFF)
CMAKE_DEPENDENT_OPTION(BEYOND_BUILD_TESTS_COVERAGE
    "Build the project with code coverage support for tests" OFF
                       "BEYOND_BUILD_TESTS" OFF)

option(BEYOND_ENABLE_CLANG_TIDY "Enable testing with clang-tidy" OFF)
option(BEYOND_ENABLE_CPPCHECK "Enable testing with cppcheck" OFF)
option(BEYOND_WARNING_AS_ERROR "Treats compiler warnings as errors" ON)

option(BEYOND_BUILD_DOCUMENTATION
    "Build the documentation of the Beyond game engine" OFF)

if (BEYOND_BUILD_DOCUMENTATION)
    include("doxygen")
endif()

include("compiler")
include("clangformat")

if (BEYOND_ENABLE_CLANG_TIDY)
include("ClangTidy")
endif()

if (BEYOND_ENABLE_CPPCHECK)
include("cppcheck")
endif()

# Conan package manager
# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.14/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake")
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(CONANFILE conanfile.txt  # or relative build/conanfile.txt
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)

if(${BEYOND_BUILD_TESTS})
enable_testing()
endif()

add_subdirectory(Engine)

