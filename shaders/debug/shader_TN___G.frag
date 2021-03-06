#version 330 core

layout(std140) uniform CommonBlock
{
   vec3 eye;
   int dummy;
   vec2 screenSize;
   vec2 cubeNearFar;
};

in vec3 N;
in vec2 uv;
in vec3 wPos;

layout (location = 0) out vec3 worldOut;
layout (location = 1) out vec3 diffuseOut;
layout (location = 2) out vec3 normalOut;

uniform sampler2D sampler;
uniform sampler2D normals;

vec3 perturb_normal( vec3 Nc, vec3 V, vec2 uv ) // from www.thetenthplanet.de/archives/1180
{
	vec3 map = texture( normals, uv ).xyz * 255.0 / 127.0 - 128.0 / 127.0;
	vec3 dp1 = dFdx( V ), dp2 = dFdy( V );
	vec2 duv1 = dFdx(uv), duv2 = dFdy(uv);
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
	float invmax = inversesqrt( max( dot( T, T ), dot( B, B ) ) );
	return normalize( mat3( T * invmax, B * invmax, Nc ) * map );
}


void main()
{
   vec4 diffuse = texture( sampler, uv );
   vec3 V = normalize( eye - wPos );
   normalOut = perturb_normal( N, -V, uv );
   diffuseOut = diffuse.rgb;
   worldOut = wPos;
}

// EOF
 