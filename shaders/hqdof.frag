#version 330

uniform sampler2D bokeh;

in vec4 pixelColor;
in vec2 uv;
out vec4 pixel;

void main() 
{
	vec4 b = texture( bokeh, uv );
	pixel = vec4( b.rgb * pixelColor.rgb * pixelColor.a, b.a );
}

// EOF