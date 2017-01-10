#version 330
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

const int MAX_GLYPHS = 180;

in GS_IN
{
    flat int id;
} gs_in [];

uniform mat4  transform;
uniform ivec4 glyph_bound [MAX_GLYPHS];
uniform vec4  glyph_uv    [MAX_GLYPHS];

out FS_IN
{
    smooth vec2 uv;
} gs_out;

void main()
{
    int id = gs_in[0].id;
    gl_Position = transform * (
        gl_in[0].gl_Position + 
        vec4(glyph_bound[id].xy, 0, 0)
        );
    gs_out.uv = glyph_uv[id].xy;
    EmitVertex();

    gl_Position = transform * (
        gl_in[0].gl_Position + 
        vec4(glyph_bound[id].xw, 0, 0)
        );
    gs_out.uv = glyph_uv[id].xw;
    EmitVertex();

    gl_Position = transform * (
        gl_in[0].gl_Position + 
        vec4(glyph_bound[id].zy, 0, 0)
        );
    gs_out.uv = glyph_uv[id].zy;
    EmitVertex();

    gl_Position = transform * (
        gl_in[0].gl_Position + 
        vec4(glyph_bound[id].zw, 0, 0)
        );
    gs_out.uv = glyph_uv[id].zw;
    EmitVertex();
}
