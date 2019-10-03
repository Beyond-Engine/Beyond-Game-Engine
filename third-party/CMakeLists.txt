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

