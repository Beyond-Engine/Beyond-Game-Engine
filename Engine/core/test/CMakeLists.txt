set(TEST_TARGET_NAME ${PROJECT_NAME}CoreTest)

add_executable(${TEST_TARGET_NAME}
    "angle_test.cpp"
    "main.cpp"
    "vector_test.cpp"
    "sparse_set_test.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

target_link_libraries(${TEST_TARGET_NAME} PRIVATE core CONAN_PKG::Catch2)

if(${BEYOND_BUILD_TESTS_COVERAGE})
    set(CMAKE_STATIC_LINKER_FLAGS "--whole-archive")
    set(CMAKE_SHARED_LINKER_FLAGS "--whole-archive")

    include("CodeCoverage")
    APPEND_COVERAGE_COMPILER_FLAGS("-g -O0 -fprofile-arcs -ftest-coverage")
    set(COVERAGE_LCOV_EXCLUDES '/usr/*' '*/.conan/*' '*/test/*' 'test/*')
    SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME coverage                    # New target name
        EXECUTABLE ${TEST_TARGET_NAME}
        DEPENDENCIES ${TEST_TARGET_NAME}  # Dependencies to build first
    )
endif()


add_test(${PROJECT_NAME}CoreTest "${CMAKE_BINARY_DIR}/bin/${TEST_TARGET_NAME}")
