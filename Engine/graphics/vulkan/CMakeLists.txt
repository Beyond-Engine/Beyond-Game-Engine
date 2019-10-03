find_package(Vulkan)

add_library(vulkan_backend "vulkan_backend.cpp")

target_link_libraries(vulkan_backend
    PRIVATE compiler_warnings
    Vulkan::Vulkan
    core platform graphics_backend
    )
