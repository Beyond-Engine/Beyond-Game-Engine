# Compiler specific settings

if(compiler_included)
  return()
endif()
set(compiler_included true)

# Link this 'library' to use the standard warnings
add_library(compiler_warnings INTERFACE)

add_library(coverage INTERFACE)
target_compile_options(coverage
                       INTERFACE
                       -fprofile-arcs
                       -ftest-coverage
                       -fno-inline
                       -fno-inline-small-functions
                       -fno-default-inline)
target_link_libraries(coverage INTERFACE gcov)

if(MSVC)
  target_compile_options(compiler_warnings INTERFACE "/permissive-")
  if(BEYOND_WARNING_AS_ERROR)
    target_compile_options(compiler_warnings INTERFACE /WX)
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(compiler_warnings
                         INTERFACE -Wall
                                   -Wextra
                                   -Wshadow
                                   -Wnon-virtual-dtor
                                   -Wold-style-cast
                                   -Wcast-align
                                   -Wunused
                                   -Woverloaded-virtual
                                   -Wconversion
                                   -Wsign-conversion
                                   -Wnull-dereference
                                   -Wdouble-promotion
                                   -Wformat=2)
  if(BEYOND_WARNING_AS_ERROR)
    target_compile_options(compiler_warnings INTERFACE -Werror)
  endif()

  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(compiler_warnings
                           INTERFACE
                           -Wno-gnu-anonymous-struct
                           -Wno-nested-anon-types
                           )
  endif()

  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(compiler_warnings
                           INTERFACE -Wmisleading-indentation
                                     -Wduplicated-cond
                                     -Wduplicated-branches
                                     -Wlogical-op
                                     -Wuseless-cast
                           )
  endif()
endif()
