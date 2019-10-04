add_library(vulkan_backend "vulkan_backend.cpp")

set(BEYOND_VULKAN_ENABLE_VALIDATION_LAYER AUTO CACHE STRING "The policy of enabling
    assertion or not in beyond game engine core.
    AUTO means follow the debug or release build setting.")
set_property(CACHE BEYOND_VULKAN_ENABLE_VALIDATION_LAYER PROPERTY STRINGS AUTO ON OFF)

if(BEYOND_VULKAN_ENABLE_VALIDATION_LAYER STREQUAL "AUTO")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(vulkan_backend PRIVATE
            BEYOND_VULKAN_ENABLE_VALIDATION_LAYER)
    endif()
elseif(BEYOND_VULKAN_ENABLE_VALIDATION_LAYER STREQUAL "ON")
    target_compile_definitions(vulkan_backend PRIVATE
        BEYOND_VULKAN_ENABLE_VALIDATION_LAYER)
endif()

target_link_libraries(vulkan_backend
    PRIVATE compiler_warnings
    core platform graphics_backend
    volk
    )

target_compile_definitions(vulkan_backend PRIVATE VK_NO_PROTOTYPES)
