#version 330

uniform sampler2D color;
uniform sampler2D depth;
uniform sampler2D dof;
uniform ivec2 screenSize;
uniform float focalDepth;

in vec2 uv;
out vec4 pixel;

void main() 
{
	float z = texture( depth, uv ).r; // range -0.1 .. -500
	vec4 d0 = texture( dof, vec2( uv.x * 0.5, uv.y ) );
	vec4 d1 = texture( dof, vec2( uv.x * 0.5 + 0.5, uv.y ) );
	vec4 d2 = vec4( 0 );
	if ((z > (focalDepth - 15)) && (z < (focalDepth + 15))) d2 = vec4( texture( color, uv ).rgb, 1 );
	float totalAlpha = d0.a + d1.a + d2.a;
	vec4 c = (d0 + d1 + d2) * (1.0f / totalAlpha);
	pixel = vec4( c.rgb, 1 );
}

// EOF