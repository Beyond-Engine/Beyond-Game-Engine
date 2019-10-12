add_library(graphics_backend INTERFACE)
target_include_directories(graphics_backend
    INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if (${BEYOND_BUILD_GRAPHICS_BACKEND_VULKAN})
  add_subdirectory(vulkan)
endif()

if (${BEYOND_BUILD_GRAPHICS_BACKEND_DX12})
    add_subdirectory(dx12)
endif()

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

if (${BEYOND_BUILD_GRAPHICS_BACKEND_VULKAN})
  target_link_libraries(graphics PRIVATE vulkan_backend)
endif()

add_executable(TestApp "main.cpp")
target_link_libraries(TestApp
    PRIVATE graphics compiler_warnings)
