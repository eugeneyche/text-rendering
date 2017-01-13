#pragma once
#include "shader.hpp"
#include <array>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

const size_t MAX_GLYPHS = 180;

struct Font
{
    GLuint atlas_tex;
    GLuint line_height;
    std::array<glm::ivec4, MAX_GLYPHS> bound;
    std::array<glm::ivec2, MAX_GLYPHS> advance;
    std::array<glm::vec4, MAX_GLYPHS> uv;
};

class FontManager
{
public:
    FontManager(ShaderManager* sm);
    virtual ~FontManager() = default;

    bool init();
    bool load_font(Font* font, int size, const char* path);
    void draw_text(
        const char* text,
        const Font* font,
        const glm::mat4& transform,
        const glm::vec4& color,
        const glm::vec2& position
        );

private:
    ShaderManager* sm_;
    GLuint program_;
    GLuint vao_;
    GLuint vbo_;
    GLint loc_transform_;
    GLint loc_color_;
    GLint loc_atlas_tex_;
    GLint loc_glyph_bound_;
    GLint loc_glyph_uv_;

    FT_Library library_;
};
