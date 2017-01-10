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
    ${GLFW_INCLUDE_DIRS}
    ${GLAD_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    )

add_executable(
    ${PROJECT_NAME}
    src/main.cpp
    src/text.cpp
    )

target_link_libraries(
    ${PROJECT_NAME}
    glfw
    glad
    ${OPENGL_gl_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    dl
    )
