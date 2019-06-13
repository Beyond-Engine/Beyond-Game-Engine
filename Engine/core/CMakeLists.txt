add_library(core
    include/beyond/core/ecs/sparse_map.hpp
    include/beyond/core/ecs/sparse_set.hpp
    include/beyond/core/utils/assert.hpp
    include/beyond/core/utils/arrow_proxy.hpp
    include/beyond/core/utils/bit_cast.hpp
    include/beyond/core/utils/crtp.hpp
    include/beyond/core/utils/expected.hpp
    include/beyond/core/utils/functional.hpp
    include/beyond/core/utils/named_type.hpp
    include/beyond/core/utils/panic.hpp
    src/utils/panic.cpp
    include/beyond/core/utils/type_traits.hpp
    include/beyond/core/math/angle.hpp
    include/beyond/core/math/constants.hpp
    include/beyond/core/math/math.hpp
    include/beyond/core/math/vector.hpp
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
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(core PRIVATE compiler_warnings
    PUBLIC
    CONAN_PKG::fmt
    CONAN_PKG::gsl_microsoft
    )

if(${BEYOND_BUILD_TESTS})
    if(${BEYOND_BUILD_TESTS_COVERAGE})
        target_link_libraries(core PUBLIC coverage)
    endif()

    add_subdirectory(test)
else()
    message("No test")
endif()


