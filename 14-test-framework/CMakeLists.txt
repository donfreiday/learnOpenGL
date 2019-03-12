# Require new CMake
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(NAME a.out)

find_package(PkgConfig REQUIRED)
find_package(OpenGL)
pkg_search_module(GLFW REQUIRED glfw3)
find_package(GLEW REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
include_directories("src")
include_directories("src/vendor")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable (${NAME} src/Renderer.cpp
src/tests/TestClearColor.cpp
src/IndexBuffer.cpp
src/VertexBuffer.cpp
src/VertexArray.cpp
src/Shader.cpp
src/vendor/stb_image/stb_image.cpp
src/vendor/imgui/imgui.cpp
src/vendor/imgui/imgui_draw.cpp
src/vendor/imgui/imgui_impl_opengl3.cpp
src/vendor/imgui/imgui_impl_glfw.cpp
src/vendor/imgui/imgui_demo.cpp
src/vendor/imgui/imgui_widgets.cpp
src/Texture.cpp
src/main.cpp)

target_link_libraries(${NAME} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})