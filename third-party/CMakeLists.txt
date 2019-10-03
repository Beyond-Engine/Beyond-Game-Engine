add_library(tl-expected INTERFACE)
target_sources(tl-expected
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/expected/tl/expected.hpp"
    )

target_include_directories(tl-expected
    INTERFACE
        $<INSTALL_INTERFACE:expected>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/expected>
)

if(${BEYOND_BUILD_VULKAN_BACKEND})
  if(${WIN32})
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
  endif()
  add_subdirectory(volk)
endif()
