cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(text_rendering)

set(CMAKE_CXX_COMPILER_ID Clang)
set(CMAKE_CXX_STANDARD 14)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(deps/glad)

find_package(Freetype REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(
    include
    ${FREETYPE_INCLUDE_DIRS}
    ${GLAD_INCLUDE_DIRS}
    )

add_executable(
    ${PROJECT_NAME}
    src/main.cpp
    src/shader.cpp
    src/font.cpp
    )

target_link_libraries(
    ${PROJECT_NAME}
    ${FREETYPE_LIBRARIES}
    glad
    glfw
    ${OPENGL_gl_LIBRARIES}
    dl
    )
