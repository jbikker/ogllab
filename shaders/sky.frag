#version 330

in vec3 uv;

out vec3 pixel;

uniform samplerCube environment;

void main()
{
    pixel = texture( environment, uv ).rgb;
}

// EOF