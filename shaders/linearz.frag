#version 330 core

uniform vec3 clipInfo;				// vec3( z_n * z_f,  z_n - z_f,  z_f )
 
out vec3 result;

float reconstructCSZ( float d ) 
{
	return clipInfo[0] / (clipInfo[1] * d + clipInfo[2]);
}

void main()
{
	result.x = reconstructCSZ( gl_FragCoord.z );
}

// EOF