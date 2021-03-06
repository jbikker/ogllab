#version 330 core

layout(std140) uniform CommonBlock
{
   vec3 eye;
   int dummy;
   vec2 screenSize;
   vec2 cubeNearFar;
};

in vec3 N;
in vec2 uv;
in vec3 wPos;

layout (location = 0) out vec3 worldOut;
layout (location = 1) out vec3 diffuseOut;
layout (location = 2) out vec3 normalOut;

uniform sampler2D sampler;


void main()
{
   vec4 diffuse = texture( sampler, uv );
   normalOut = normalize( N );
   diffuseOut = diffuse.rgb;
   worldOut = wPos;
}

// EOF
 