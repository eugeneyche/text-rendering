#pragma once
#include <glad/glad.h>
#include <vector>

GLuint make_shader(GLenum type, const char* path);
GLuint make_program(const std::vector<GLuint>& parts);

