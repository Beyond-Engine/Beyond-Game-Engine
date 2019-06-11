conan_cmake_run(REQUIRES glfw/3.3@bincrafters/stable
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)

add_executable(TestApp "main.cpp")
target_link_libraries(TestApp PRIVATE CONAN_PKG::fmt CONAN_PKG::glfw)
