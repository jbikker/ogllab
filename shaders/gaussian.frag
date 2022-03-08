#version 330

#define EDGE_SHARPNESS			1.0		// increase to make depth edges crisper; decrease to reduce flicker
#define R						6		// filter radius in pixels; will be multiplied by SCALE

float gaussian[R+1] = float[](0.111220, 0.107798, 0.098151, 0.083953, 0.067458, 0.050920, 0.036108);

uniform sampler2D				source;
uniform sampler2D				depth;

uniform ivec2					axis;

out vec3						pixel;

void main() 
{
	ivec2 ssC = ivec2( gl_FragCoord.xy );
	vec3 temp = texelFetch( source, ssC, 0 ).rgb;
	vec3 key = texelFetch( depth, ssC, 0 ).xyz;
	vec3 sum = temp;
	if (key == 1.0) { pixel = sum; return; }
	float BASE = gaussian[0], totalWeight = BASE;
	sum *= totalWeight;
	for( int r = -R; r <= R; ++r ) if (r != 0) 
	{
		vec3 value = texelFetch( source, ssC + axis * r, 0 ).rgb;
		vec3 tapKey = texelFetch( depth, ssC + axis * r, 0 ).xyz;
		float weight = 0.3 + gaussian[abs( r )];
		// weight *= max( 0.0, 1.0 - (EDGE_SHARPNESS * 2000.0) * abs( tapKey - key ) );
		weight *= max( 0.0, 1.0 - 8.0 * length( key - tapKey ) );
		sum += value * weight, totalWeight += weight;
	}
	const float epsilon = 0.0001;
	pixel = sum / (totalWeight + epsilon);
}

// EOF