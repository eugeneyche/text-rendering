#include <array>
#include <glad/glad.h>
#include <glm/glm.hpp>

const size_t MAX_GLYPHS = 180;

struct Font
{
    GLuint atlas;
    GLuint line_height;
    std::array<glm::ivec4, MAX_GLYPHS> bound;
    std::array<glm::ivec2, MAX_GLYPHS> advance;
    std::array<glm::vec4, MAX_GLYPHS> uv;
};

bool load_font(Font* font, const char* path, int size);

bool init_text_technique();
void draw_text(
    const glm::mat4& projection,
    Font* font,
    const glm::vec2 position,
    const glm::vec4& color,
    const char* text
    );
