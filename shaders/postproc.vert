#version 330

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 tuv;

uniform mat4 view;
uniform vec2 screenSize;

out vec2 uv;
out vec2 sxy;

void main()
{
	uv = tuv;
	sxy = tuv * screenSize;
	gl_Position = view * pos;
}

// EOF