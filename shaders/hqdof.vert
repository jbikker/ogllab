#version 330

uniform sampler2D color;
uniform sampler2D depth;
uniform ivec2 screenSize;
uniform float focalDepth;

out vec4 pixelColor;
out vec2 uv;

#define YDISP	-0.15

float vx[3] = float[]( -0.3, -0.3, 1 );
float vy[3] = float[]( -1 + YDISP, 0.6 + YDISP, 0.6 + YDISP );
float vu[3] = float[]( 0, 0, 1 );
float vv[3] = float[]( 0, 1, 1 );

void main()
{
	// get triangle and vertex number from index
	int idx = gl_VertexID, triangle = idx / 3, vertex = idx % 3;
	// calculate draw position
	float sx = float( triangle % screenSize.x ) / float( screenSize.x ); // 0..1
	float sy = float( triangle / screenSize.x ) / float( screenSize.y ); // 0..1
	// calculate distance to focal plane
	float z = texture( depth, vec2( sx, sy ) ).r; // range -0.1 .. -500
	float distance = abs( focalDepth - z );
	// calculate CoC
	float radius = max( 2.0, min( 20.0, abs( distance ) ) );
	float area = 3.141592 * radius * radius;
	vec2 texel = vec2( 1.0 / float( screenSize.x ), 1.0 / (screenSize.y ) );
	vec2 relPos = texel * radius;
	// fetch source color
	pixelColor = vec4( texture( color, vec2( sx, sy ) ).rgb, 16.0 / area );
	// output
#if 0
	float nsx = sx - 1.0;
	float nsy = sy * 2.0 - 1.0;
	gl_Position = vec4( nsx + relPos.x * vx[vertex] * 0.5, nsy + relPos.y * vy[vertex], 0, 1 );
	uv = vec2( vu[vertex], vv[vertex] );
#else
	if (z < (focalDepth - 15))
	{
		// far DOF; render to left side of low-res output
		float nsx = sx - 1.0;
		float nsy = sy * 2.0 - 1.0;
		gl_Position = vec4( nsx + relPos.x * vx[vertex] * 0.5, nsy + relPos.y * vy[vertex], 0, 1 );
		uv = vec2( vu[vertex], vv[vertex] );
	}
	else if (z > (focalDepth + 15))
	{
		// near DOF; render to right side of low-res output
		float nsx = sx;
		float nsy = sy * 2.0 - 1.0;
		gl_Position = vec4( nsx + relPos.x * vx[vertex] * 0.5, nsy + relPos.y * vy[vertex], 0, 1 );
		uv = vec2( vu[vertex], vv[vertex] );
	}
	else
	{
		// in focus; export off-screen position to kill triangle
		gl_Position = vec4( 2, 0, 0, 1 );
	}
#endif
}

// EOF