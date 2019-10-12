add_library(tl-expected INTERFACE)
target_sources(tl-expected
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/expected/tl/expected.hpp"
    )

target_include_directories(tl-expected SYSTEM
    INTERFACE
        $<INSTALL_INTERFACE:expected>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/expected>
)

set(LIB_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

if(${BEYOND_BUILD_GRAPHICS_BACKEND_VULKAN})
    # Volk
    if(WIN32)
       set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
    endif()
    add_subdirectory(volk)

    add_library(vma INTERFACE)
    target_compile_definitions(vma INTERFACE
        VMA_STATIC_VULKAN_FUNCTIONS=1)
    target_include_directories(vma SYSTEM
        INTERFACE
            $<INSTALL_INTERFACE:VulkanMemoryAllocator/src>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/src>
    )
endif()
