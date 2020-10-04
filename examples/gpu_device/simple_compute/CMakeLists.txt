add_executable(SimpleCompute "main.cpp")
target_link_libraries(SimpleCompute
        PRIVATE graphics compiler_warnings)
if (${BEYOND_BUILD_GRAPHICS_BACKEND_VULKAN})
    add_dependencies(SimpleCompute vkshader)
endif ()
