#include "shader.hpp"
#include "text.hpp"
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

struct Glyph
{
    GLint id;
    glm::vec2 position;
};

struct TextRenderingTechnique
{
    GLuint program;
    GLuint vao;
    GLuint vbo;
    GLint loc_transform;
    GLint loc_color;
    GLint loc_atlas_tex;
    GLint loc_glyph_bound;
    GLint loc_glyph_uv;
} tech;

bool init_text_rendering()
{
    GLuint vert = make_shader(GL_VERTEX_SHADER, "shaders/text.vert");
    GLuint geom = make_shader(GL_GEOMETRY_SHADER, "shaders/text.geom");
    GLuint frag = make_shader(GL_FRAGMENT_SHADER, "shaders/text.frag");

    tech.program = make_program({vert, geom, frag}); 

    glDeleteShader(vert);
    glDeleteShader(geom);
    glDeleteShader(frag);

    tech.loc_transform = glGetUniformLocation(tech.program, "transform");
    tech.loc_color = glGetUniformLocation(tech.program, "color");
    tech.loc_atlas_tex = glGetUniformLocation(tech.program, "atlas_tex");
    tech.loc_glyph_bound = glGetUniformLocation(tech.program, "glyph_bound");
    tech.loc_glyph_uv = glGetUniformLocation(tech.program, "glyph_uv");

    glGenVertexArrays(1, &tech.vao);
    glGenBuffers(1, &tech.vbo);

    glBindVertexArray(tech.vao);
    glBindBuffer(GL_ARRAY_BUFFER, tech.vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_INT, sizeof(Glyph), reinterpret_cast<GLvoid*>(offsetof(Glyph, id)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Glyph), reinterpret_cast<GLvoid*>(offsetof(Glyph, position)));

    return true;
}

bool make_font(Font* font, const char* path, int size)
{
    FT_Library library;
    if (FT_Init_FreeType(&library)) {
        fprintf(stderr, "Failed to initialize Free Type library.\n");
        return false;
    }
    FT_Face face;
    if (FT_New_Face(library, path, 0, &face)) {
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


void draw_text(
        Font* font,
        const glm::mat4& transform,
        const glm::vec4& color,
        const glm::vec2& position,
        const char* text
        )
{
    glUseProgram(tech.program);

    glUniformMatrix4fv(tech.loc_transform, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform4fv(tech.loc_color, 1, glm::value_ptr(color));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, font->atlas_tex);
    glUniform1i(tech.loc_atlas_tex, 1);
    glUniform4iv(tech.loc_glyph_bound, MAX_GLYPHS, reinterpret_cast<GLint*>(font->bound.data()));
    glUniform4fv(tech.loc_glyph_uv, MAX_GLYPHS, reinterpret_cast<GLfloat*>(font->uv.data()));

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

    glBindVertexArray(tech.vao);
    glBindBuffer(GL_ARRAY_BUFFER, tech.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Glyph) * n_glyphs, glyphs.data(), GL_DYNAMIC_DRAW);

    glUseProgram(tech.program);
    glDrawArrays(GL_POINTS, 0, n_glyphs);
}
