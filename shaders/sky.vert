#version 330

layout (location = 0) in vec4 pos;

uniform mat4 view;

out vec3 uv;

void main()
{
    vec4 P = view * pos;
    gl_Position = P.xyww;
    uv = normalize( pos.xyz );
}

// EOF