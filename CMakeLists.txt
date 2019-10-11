cmake_minimum_required(VERSION 3.12)

project(BeyondGameEngine VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(ExternalProject)
include("CMakeDependentOption")

link_directories(${CMAKE_CURRENT_BINARY_DIR}/lib)

# Organize unrelated targets to clean IDE hierarchy.
set(DEPENDENCY_FOLDER "third-party")

# This will also clean up the CMake ALL_BUILD, INSTALL, RUN_TESTS and ZERO_CHECK projects.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER ${DEPENDENCY_FOLDER})

# Output binary to predictable location.
set(BINARY_OUT_DIR ${CMAKE_BINARY_DIR}/bin)
set(LIB_OUT_DIR ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_OUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUT_DIR})

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BINARY_OUT_DIR})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_OUT_DIR})
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_OUT_DIR})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

option(BEYOND_BUILD_TESTS "Builds the tests" OFF)
CMAKE_DEPENDENT_OPTION(BEYOND_BUILD_TESTS_COVERAGE
    "Build the project with code coverage support for tests" OFF
                       "BEYOND_BUILD_TESTS" OFF)

option(BEYOND_ENABLE_CLANG_TIDY "Enable testing with clang-tidy" OFF)
option(BEYOND_ENABLE_CPPCHECK "Enable testing with cppcheck" OFF)
option(BEYOND_WARNING_AS_ERROR "Treats compiler warnings as errors" ON)

option(BEYOND_BUILD_DOCUMENTATION
    "Build the documentation of the Beyond game engine" OFF)

option(BEYOND_BUILD_VULKAN_BACKEND "Build the vulkan backend" ON)

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
conan_check(VERSION 1.0.0 REQUIRED)

conan_cmake_run(CONANFILE conanfile.txt  # or relative build/conanfile.txt
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)

if(${BEYOND_BUILD_TESTS})
enable_testing()
endif()

add_subdirectory(third-party)
add_subdirectory(Engine)

