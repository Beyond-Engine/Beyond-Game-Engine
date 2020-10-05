add_library(d3d12_backend
    "include/beyond/d3d12/d3d12_interface.hpp"
    "src/d3d12_device.cpp")

target_link_libraries(d3d12_backend
    PRIVATE compiler_warnings
    core platform graphics_backend
    d3d12.lib
    )

target_include_directories(d3d12_backend
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/beyond/d3d12
)