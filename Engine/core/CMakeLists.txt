add_library(core
    include/core/assert.hpp
    include/core/arrow_proxy.hpp
    include/core/bit_cast.hpp
    include/core/crtp.hpp
    include/core/panic.hpp
    src/panic.cpp
    include/core/type_traits.hpp
    include/core/ecs/sparse_map.hpp
    include/core/ecs/sparse_set.hpp
    include/core/math/angle.hpp
    include/core/math/constants.hpp
    include/core/math/math.hpp
    include/core/math/vector.hpp
)

set(BEYOND_CORE_ASSERT_POLICY AUTO CACHE STRING "The policy of enabling
    assertion or not in beyond game engine core.
    AUTO means follow the debug or release build setting.")
set_property(CACHE BEYOND_CORE_ASSERT_POLICY PROPERTY STRINGS AUTO ON OFF)

if(BEYOND_CORE_ASSERT_POLICY STREQUAL "AUTO")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(core PUBLIC BEYOND_GAME_ENGINE_CORE_ENABLE_ASSERT)
    endif()
elseif(BEYOND_CORE_ASSERT_POLICY STREQUAL "ON")
    target_compile_definitions(core PUBLIC BEYOND_GAME_ENGINE_CORE_ENABLE_ASSERT)
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


