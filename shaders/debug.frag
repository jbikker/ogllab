#version 330

uniform sampler2D data;

out vec3 pixel;

void main()
{
	float AO = texelFetch( data, ivec2( gl_FragCoord.xy ), 0 ).x;
	AO = max( 0, AO ); // (AO + 1) * 0.5;
	pixel = vec3( AO, AO, AO );
}

// EOF