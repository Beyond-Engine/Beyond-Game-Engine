set(TEST_TARGET_NAME beyond_graphics_test)

add_executable(${TEST_TARGET_NAME}
        "backend/mock_gpu_device.hpp"
        "backend/mapping_test.cpp"
        "main.cpp"
        )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

target_link_libraries(${TEST_TARGET_NAME} PRIVATE graphics CONAN_PKG::Catch2)

add_test(TEST_TARGET_NAME "${CMAKE_BINARY_DIR}/bin/${TEST_TARGET_NAME}")
