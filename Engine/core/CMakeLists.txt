add_library(core
    include/core/assert.hpp
    include/core/bit_cast.hpp
    include/core/type_traits.hpp
    include/core/math/angle.hpp
    include/core/math/constants.hpp
    include/core/math/vector.hpp
    src/math/vector.cpp)

if(BEYOND_GAME_ENGINE_CORE_DEBUG)
    target_compile_definitions(core PUBLIC BEYOND_GAME_ENGINE_CORE_DEBUG)
endif()

target_include_directories(core
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Engine/core/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(core PRIVATE compiler_warnings
    PUBLIC
    CONAN_PKG::fmt
    )

if(${BEYOND_BUILD_TESTS})
    if(${BEYOND_BUILD_TESTS_COVERAGE})
        target_compile_options(core PUBLIC -fprofile-arcs -ftest-coverage)
        target_link_libraries(core PUBLIC gcov)
    endif()

    add_subdirectory(test)
else()
    message("No test")
endif()


