#version 330

uniform sampler2D wpos;
uniform sampler2D normal;
uniform samplerBuffer data;

in vec2 uv;

out vec3 pixel;

uint hash( uint x ) 
{
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

float floatConstruct( uint m ) 
{
	const uint a = 0x007FFFFFu, b = 0x3F800000u;
	m &= a, m |= b;
	return uintBitsToFloat( m ) - 1.0;
}

float random( float x ) { return floatConstruct( hash( floatBitsToUint( x ) ) ); }
float random( vec2  v ) { return floatConstruct( hash( floatBitsToUint( v ) ) ); }
float random( vec3  v ) { return floatConstruct( hash( floatBitsToUint( v ) ) ); }
float random( vec4  v ) { return floatConstruct( hash( floatBitsToUint( v ) ) ); }

vec3 Shade( const vec3 _P, const vec3 _N )
{
	// initialize random number generation
	float seed = gl_FragCoord.x + 123 * gl_FragCoord.y;
	// initialize output
	vec3 c = vec3( 0 );
	// loop over 6 light triangles
	for ( int i = 0; i < 6; i++ )
	{
		// fetch light triangle vertices
		vec4 v0 = texelFetch( data, i * 3 + 0 );
		vec4 v1 = texelFetch( data, i * 3 + 1 );
		vec4 v2 = texelFetch( data, i * 3 + 2 );
		vec3 emissive = 6 * vec3( v0.a, v1.a, v2.a );
		vec3 LN = cross( normalize( v1.xyz - v0.xyz ), normalize( v2.xyz - v0.xyz ) );
		float l = 0;
		for ( int j = 0; j < 8; j++ )
		{
			// get random point on triangle - source: Kenneth Gorking, devmaster
			float r0 = seed = random( seed );
			float r1 = seed = random( seed );
			float b0 = r0;
			float b1 = (1.0 - b0) * r1;
			float b2 = 1 - b0 - b1;
			vec3 P = v0.xyz * b0 + v1.xyz * b1 + v2.xyz * b2;
			// shade
			vec3 L = P - _P;
			float att = 1.0 / dot( L, L );
			L = normalize( L );
			l += max( 0, dot( _N, L ) ) * max( 0, dot( LN, -L ) ) * att;
		}
		c += l * emissive;
	}
	return c;
}

void main()
{
	vec3 N = texture( normal, uv ).xyz;
	vec3 P = texture( wpos, uv ).xyz;
	pixel = Shade( P, N );
}

// EOF