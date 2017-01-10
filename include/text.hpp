#pragma once
#include <array>
#include <glad/glad.h>
#include <glm/glm.hpp>

const size_t MAX_GLYPHS = 180;

struct Font
{
    GLuint atlas_tex;
    GLuint line_height;
    std::array<glm::ivec4, MAX_GLYPHS> bound;
    std::array<glm::ivec2, MAX_GLYPHS> advance;
    std::array<glm::vec4, MAX_GLYPHS> uv;
};

bool init_text_rendering();
bool make_font(Font* font, const char* path, int size);
void draw_text(
    Font* font,
    const glm::mat4& transform,
    const glm::vec4& color,
    const glm::vec2& position,
    const char* text
    );
