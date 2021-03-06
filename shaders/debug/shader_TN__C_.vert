#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 vn;
layout(location = 2) in vec2 vuv;

layout(std140) uniform TransformBlock
{
   mat4 view;
   mat4 world;
};


out vec3 N;
out vec2 uv;
out vec3 wPos;

void main()
{
   gl_Position = view * pos;
   wPos = (world * pos).xyz;
   uv = vuv;
   N = (world * vec4( vn, 0 )).xyz;
}

// EOF
 