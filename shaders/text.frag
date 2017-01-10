#version 330

in FS_IN {
    smooth vec2 uv;
} fs_in;

uniform vec4 color;
uniform sampler2D atlas_tex;

out vec4 fs_out;

void main()
{
    fs_out = vec4(color.rgb, color.a * texture(atlas_tex, fs_in.uv).r);
}
