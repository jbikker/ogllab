#version 330 core 
 
in vec4 pos;

layout(std140) uniform TransformBlock
{
	mat4 view;
	mat4 world;
};

out vec4 wPos;

void main() 
{ 
	wPos = pos;
	gl_Position = world * pos;
}

// EOF