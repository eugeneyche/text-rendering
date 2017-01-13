#include "font.hpp"
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Glyph
{
    GLint id;
    glm::vec2 position;
};

FontManager::FontManager(ShaderManager* sm)
  : sm_ {sm}
{
}

bool FontManager::init()
{
    GLuint vert = sm_->make_shader(GL_VERTEX_SHADER, "shaders/text.vert");
    GLuint geom = sm_->make_shader(GL_GEOMETRY_SHADER, "shaders/text.geom");
    GLuint frag = sm_->make_shader(GL_FRAGMENT_SHADER, "shaders/text.frag");
    program_ = sm_->make_program({vert, geom, frag}); 

    glDeleteShader(vert);
    glDeleteShader(geom);
    glDeleteShader(frag);

    loc_transform_ = glGetUniformLocation(program_, "transform");
    loc_color_ = glGetUniformLocation(program_, "color");
    loc_atlas_tex_ = glGetUniformLocation(program_, "atlas_tex");
    loc_glyph_bound_ = glGetUniformLocation(program_, "glyph_bound");
    loc_glyph_uv_ = glGetUniformLocation(program_, "glyph_uv");

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_INT, sizeof(Glyph), reinterpret_cast<GLvoid*>(offsetof(Glyph, id)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Glyph), reinterpret_cast<GLvoid*>(offsetof(Glyph, position)));

    if (FT_Init_FreeType(&library_)) {
        fprintf(stderr, "Failed to initialize Free Type library_.\n");
        return false;
    }

    return true;
}

bool FontManager::load_font(Font* font, int size, const char* path)
{
    FT_Face face;
    if (FT_New_Face(library_, path, 0, &face)) {
        fprintf(stderr, "Failed to load font \"%s\".\n", path);
        return false;
    }
    FT_Set_Pixel_Sizes(face, 0, size);
    font->line_height = face->size->metrics.height;

    std::vector<GLubyte> glyph_buffer;
    std::array<glm::ivec2, MAX_GLYPHS> glyph_size;
    std::array<size_t, MAX_GLYPHS> glyph_buffer_end_offset;

    int n_glyphs = 0;
    glm::ivec2 max_glyph_size = {0, 0};

    for (int c = 0; c < MAX_GLYPHS; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            font->bound[c] = {0, 0, 0, 0};
            font->advance[c] = {0, 0};
            font->uv[c] = {0.f, 0.f, 0.f, 0.f};
            glyph_size[c] = {0, 0};
            glyph_buffer_end_offset[c] = 0u;
            continue;
        }

        if (isspace(c)) {
            font->bound[c] = {0, 0, 0, 0};
            font->advance[c] = {
                face->glyph->advance.x,
                face->glyph->advance.y,
            };
            font->uv[c] = {0.f, 0.f, 0.f, 0.f};
            glyph_size[c] = {0, 0};
            glyph_buffer_end_offset[c] = 0u;
            continue;
        }

        n_glyphs++;
        size_t bitmap_size = face->glyph->bitmap.width * face->glyph->bitmap.rows;
        font->bound[c] = {
            face->glyph->bitmap_left,
            face->glyph->bitmap_top - face->glyph->bitmap.rows,
            face->glyph->bitmap_left + face->glyph->bitmap.width,
            face->glyph->bitmap_top
        };

        font->advance[c] = {
            face->glyph->advance.x,
            face->glyph->advance.y,
        };
        glyph_buffer.insert(
            glyph_buffer.end(),
            face->glyph->bitmap.buffer,
            face->glyph->bitmap.buffer + bitmap_size
            );
        glyph_size[c] = {face->glyph->bitmap.width, face->glyph->bitmap.rows};
        glyph_buffer_end_offset[c] = glyph_buffer.size();
        if (max_glyph_size.x < face->glyph->bitmap.width) {
            max_glyph_size.x = face->glyph->bitmap.width;
        }
        if (max_glyph_size.y < face->glyph->bitmap.rows) {
            max_glyph_size.y = face->glyph->bitmap.rows;
        }
    }

    int n_rows = 4;
    int n_glyphs_per_row = (n_glyphs - 1) / n_rows + 1;
    int tex_width = n_glyphs_per_row * max_glyph_size.y;
    int tex_height = n_rows * max_glyph_size.y;

    std::vector<GLubyte> texture_buffer (tex_width * tex_height);
    int glyph_id = 0;
    for (int c = 0; c < MAX_GLYPHS; c++) {
        if (glyph_size[c].x == 0) {
            continue;
        }
        int tex_x = (glyph_id % n_glyphs_per_row) * max_glyph_size.x;
        int tex_y = (glyph_id / n_glyphs_per_row) * max_glyph_size.y;
        int glyph_row_offset = glyph_buffer_end_offset[c];
        for (int y = 0; y < glyph_size[c].y; y++) {
            glyph_row_offset -= glyph_size[c].x;
            memcpy(
                texture_buffer.data() + ((tex_y + y) * tex_width + tex_x),
                glyph_buffer.data() + glyph_row_offset,
                glyph_size[c].x
                );
        }
        font->uv[c].x = static_cast<float>(tex_x) / tex_width;
        font->uv[c].y = static_cast<float>(tex_y) / tex_height;
        font->uv[c].z = static_cast<float>(tex_x + glyph_size[c].x) / tex_width;
        font->uv[c].w = static_cast<float>(tex_y + glyph_size[c].y) / tex_height;
        glyph_id++;
    }

    glGenTextures(1, &font->atlas_tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->atlas_tex);
    glTexImage2D(
        GL_TEXTURE_2D, 
        0, GL_RED, tex_width, tex_height, 
        0, GL_RED, GL_UNSIGNED_BYTE, 
        texture_buffer.data()
        );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return true;
}


void FontManager::draw_text(
        const char* text,
        const Font* font,
        const glm::mat4& transform,
        const glm::vec4& color,
        const glm::vec2& position
        )
{
    glUseProgram(program_);

    glUniformMatrix4fv(loc_transform_, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform4fv(loc_color_, 1, glm::value_ptr(color));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font->atlas_tex);
    glUniform1i(loc_atlas_tex_, 1);
    glUniform4iv(loc_glyph_bound_, MAX_GLYPHS, reinterpret_cast<const GLint*>(font->bound.data()));
    glUniform4fv(loc_glyph_uv_, MAX_GLYPHS, reinterpret_cast<const GLfloat*>(font->uv.data()));

    int text_len = strlen(text);

    glm::vec2 line_cursor = position;
    glm::vec2 cursor = line_cursor;
    std::vector<Glyph> glyphs (text_len);

    int n_glyphs = 0;
    for (int i = 0; i < text_len; i++) {
        int id = text[i];
        if (id == '\n') {
            line_cursor.y -= font->line_height / 64.f;
            cursor = line_cursor;
            continue;
        }
        glyphs[n_glyphs].id = id;
        glyphs[n_glyphs].position = cursor;
        cursor.x += font->advance[id].x / 64.f;
        cursor.y -= font->advance[id].y / 64.f;
        n_glyphs++;
    }

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Glyph) * n_glyphs, glyphs.data(), GL_DYNAMIC_DRAW);

    glUseProgram(program_);
    glDrawArrays(GL_POINTS, 0, n_glyphs);
}
