#version 330

layout (location = 0) in vec4 pos;

uniform mat4 view;
uniform mat4 world;

out vec3 uv;
out vec3 wPos;

void main()
{
    vec4 P = view * pos;
    gl_Position = P.xyww;
    wPos = (world * pos).xyz;
    uv = pos.xyz;
}

// EOF