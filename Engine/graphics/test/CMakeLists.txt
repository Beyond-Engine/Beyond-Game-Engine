set(TEST_TARGET_NAME beyond_graphics_test)

add_executable(${TEST_TARGET_NAME}
    "backend/mock_backend.hpp"
    "backend/mapping_ptr_test.cpp"
    "main.cpp"
    )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

target_link_libraries(${TEST_TARGET_NAME} PRIVATE graphics CONAN_PKG::Catch2)

#if(${BEYOND_BUILD_TESTS_COVERAGE})
#    set(CMAKE_STATIC_LINKER_FLAGS "--whole-archive")
#    set(CMAKE_SHARED_LINKER_FLAGS "--whole-archive")

#    include("CodeCoverage")
#    target_link_libraries(${TEST_TARGET_NAME} PRIVATE coverage)
#    set(COVERAGE_LCOV_EXCLUDES '/usr/*' '*/.conan/*' '*/test/*' 'test/*' '*/src/utils/panic.cpp')
#    SETUP_TARGET_FOR_COVERAGE_LCOV(
#        NAME ${PROJECT_NAME}CoreTestCoverage# New target name
#        EXECUTABLE ${TEST_TARGET_NAME}
#        DEPENDENCIES ${TEST_TARGET_NAME}  # Dependencies to build first
#    )
#endif()


add_test(TEST_TARGET_NAME "${CMAKE_BINARY_DIR}/bin/${TEST_TARGET_NAME}")
