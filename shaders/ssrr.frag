#version 330

uniform vec4 projInfo;
uniform sampler2D depth;
uniform sampler2D color;

in vec2 uv;

out vec4 pixel;

vec3 reconstructCSPosition( vec2 S, float z )	{ return vec3( (S * projInfo.xy + projInfo.zw) * z, z ); }
vec3 reconstructCSFaceNormal( vec3 P )			{ return normalize( cross( dFdx( P ), dFdy( P ) ) ); }

// get camera space position for pixel centre and depth buffer value
vec3 getCSPosition( ivec2 screenPos, float z ) 
{
	vec3 P;
	P.z = z;
	P = reconstructCSPosition( vec2( screenPos ) + vec2( 0.5 ), P.z );
	return P;
}

// get screen space position for camera space position
ivec2 getSSPosition( vec3 P )
{
	vec2 screenPos = (P.xy / P.z - projInfo.zw) / projInfo.xy;
	return ivec2( screenPos );	
}

void main()
{
	ivec2 screenPos = ivec2( gl_FragCoord.xy );
	ivec2 screenSize = textureSize( color, 0 );
	pixel = texelFetch( color, screenPos, 0 );
	if (pixel.a < 0.1)
	{
		float z = texelFetch( depth, screenPos, 0 ).r;
		vec3 P = getCSPosition( screenPos, z );
		vec3 N = reconstructCSFaceNormal( P );
		vec3 V = normalize( P );
		vec3 R = reflect( V, N );
		// trace
		vec3 O = P + 0.05 * R;
		float scale = 1.0;
		vec2 sv = (R.xy / O.z) / projInfo.xy;
		float stepSize = 5.0 / max( abs( sv.x ), abs( sv.y ) );
		for( int i = 0; i < 64; ++i, scale *= 0.925 )
		{
			// advance	
			O += stepSize * R;
			// is current point behind geometry?
			screenPos = getSSPosition( O );
			if ((screenPos.x < 0) || (screenPos.x > screenSize.x) || (screenPos.y < 0) || (screenPos.y > screenSize.y)) break;
			float z = texelFetch( depth, screenPos, 0 ).r;
			if (z > O.z)
			{
				if (i == 0) break;
				// refine
				float direction = -1.0;
				stepSize *= 0.5;
				for( int j = 0; j < 3; ++j )
				{
					O += R * stepSize * direction;
					screenPos = getSSPosition( O );
					float z = texelFetch( depth, screenPos, 0 ).r;
					if ((z * direction) > (O.z * direction)) direction *= -1.0;
					stepSize *= 0.5;
				}
				if (abs( z - O.z) < 0.25) pixel += scale * texelFetch( color, screenPos, 0 );
				break;
			}
		}
	}
	pixel.a = 1.0;
}

// EOF