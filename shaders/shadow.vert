#version 330 core

in vec4 pos;

layout(std140) uniform TransformBlock
{
	mat4 view;
	mat4 world;
};

void main()
{
	gl_Position = view * pos;
}

// EOF