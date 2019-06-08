set(TEST_TARGET_NAME ${PROJECT_NAME}CoreTest)

add_executable(${TEST_TARGET_NAME}
    "main.cpp"
    "containers/array_view_test.cpp"
    "ecs/sparse_set_test.cpp"
    "ecs/sparse_map_test.cpp"
    "math/angle_test.cpp"
    "math/functions_test.cpp"
    "math/vector_test.cpp"
    "utils/expected_test.cpp"
    "utils/named_type_test.cpp"
    "utils/functional_test.cpp"
    )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

target_link_libraries(${TEST_TARGET_NAME} PRIVATE core CONAN_PKG::Catch2)

if(${BEYOND_BUILD_TESTS_COVERAGE})
    set(CMAKE_STATIC_LINKER_FLAGS "--whole-archive")
    set(CMAKE_SHARED_LINKER_FLAGS "--whole-archive")

    include("CodeCoverage")
    target_link_libraries(${TEST_TARGET_NAME} PRIVATE coverage)
    set(COVERAGE_LCOV_EXCLUDES '/usr/*' '*/.conan/*' '*/test/*' 'test/*' '*/src/utils/panic.cpp')
    SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME ${PROJECT_NAME}CoreTestCoverage# New target name
        EXECUTABLE ${TEST_TARGET_NAME}
        DEPENDENCIES ${TEST_TARGET_NAME}  # Dependencies to build first
    )
endif()


add_test(${PROJECT_NAME}CoreTest "${CMAKE_BINARY_DIR}/bin/${TEST_TARGET_NAME}")
