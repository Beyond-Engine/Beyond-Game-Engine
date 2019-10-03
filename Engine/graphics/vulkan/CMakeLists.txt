find_package(Vulkan)

add_library(vulkan_backend "vulkan_backend.cpp")

target_link_libraries(vulkan_backend
    PRIVATE compiler_warnings
    core platform graphics_backend
    volk
    )

target_compile_definitions(vulkan_backend PRIVATE VK_NO_PROTOTYPES)
