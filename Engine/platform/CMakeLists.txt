set(BEYOND_PLATFORM_GLFW ON)

add_library(platform)
target_sources(platform
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include/beyond/platform/platform.hpp"
    PRIVATE
        $<$<BOOL:${BEYOND_PLATFORM_GLFW}>:src/glfw_platform_impl.cpp>
    )

target_include_directories(platform
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(platform PUBLIC core)

if (${BEYOND_PLATFORM_GLFW})
    conan_cmake_run(REQUIRES glfw/3.3@bincrafters/stable
                    BASIC_SETUP CMAKE_TARGETS
                    BUILD missing)

    target_link_libraries(platform
        PRIVATE CONAN_PKG::glfw)
endif()

