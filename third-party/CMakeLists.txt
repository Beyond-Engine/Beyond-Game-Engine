if (${BEYOND_BUILD_GRAPHICS_BACKEND_VULKAN})
    # Volk
    if (WIN32)
        set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
    endif ()
    add_subdirectory(volk)

    add_library(vma INTERFACE)
    target_compile_definitions(vma INTERFACE
            VMA_STATIC_VULKAN_FUNCTIONS=1)
    target_include_directories(vma SYSTEM
            INTERFACE
            $<INSTALL_INTERFACE:VulkanMemoryAllocator/src>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/src>
            )
endif ()
