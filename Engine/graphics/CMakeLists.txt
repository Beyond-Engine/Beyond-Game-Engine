add_library(graphics_backend INTERFACE)
target_include_directories(graphics_backend
    INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if (${BEYOND_BUILD_VULKAN_BACKEND})
  add_subdirectory(vulkan)
  add_executable(TestApp "main.cpp")
  target_link_libraries(TestApp
    PRIVATE core platform compiler_warnings graphics_backend vulkan_backend)
endif()
