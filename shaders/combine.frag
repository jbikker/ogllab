#version 330

uniform sampler2D diffuse;
uniform sampler2D light;

in vec2 uv;

out vec3 pixel;

void main()
{
	vec3 c = texture( diffuse, uv ).xyz;
	vec3 l = texture( light, uv ).xyz;
	vec3 linear = (c * l + c * vec3( 0.05, 0.05, 0.04 ) * c) * 2.5;
	pixel = vec3( sqrt( linear.r ), sqrt( linear.g ), sqrt( linear.b ) );
}

// EOF