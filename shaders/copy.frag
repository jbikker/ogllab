#version 330

uniform sampler2D color;

in vec2 uv;
out vec3 pixel;

void main()
{
	pixel = texture( color, uv ).rgb;
}

// EOF