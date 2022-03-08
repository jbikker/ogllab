#version 400

layout (location = 0) in vec4 pos;

uniform mat4 view;

void main()
{
	gl_Position = view * pos;
}


// EOF