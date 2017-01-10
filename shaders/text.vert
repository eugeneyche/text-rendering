#version 330
layout(location = 0) in int id;
layout(location = 1) in vec2 position;

out GS_IN
{
    flat int id;
} vs_out;

void main()
{
    vs_out.id = id;
    gl_Position = vec4(position, 0, 1);
}
