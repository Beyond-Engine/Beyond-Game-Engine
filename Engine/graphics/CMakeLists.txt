add_library(graphics_backend INTERFACE)
target_include_directories(graphics_backend
        INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )

if (${BEYOND_BUILD_GRAPHICS_BACKEND_VULKAN})
    add_subdirectory(vulkan)
endif ()

if (${BEYOND_BUILD_GRAPHICS_BACKEND_D3D12})
    add_subdirectory(d3d12)
endif ()

add_library(graphics
        "include/beyond/graphics/backend.hpp"
        "src/backend.cpp")
target_include_directories(graphics
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(graphics
        PUBLIC core platform
        PRIVATE
        compiler_warnings
        )

if (${BEYOND_BUILD_GRAPHICS_BACKEND_D3D12})
    target_link_libraries(graphics PRIVATE d3d12_backend)
endif ()

if (${BEYOND_BUILD_GRAPHICS_BACKEND_VULKAN})
    target_link_libraries(graphics PRIVATE vulkan_backend)
endif ()


if (${BEYOND_BUILD_TESTS})
    add_subdirectory(test)
endif ()