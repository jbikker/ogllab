#version 330 core

in vec3 N;
in vec2 uv;
in vec3 wPos;

out vec4 pixel;

struct PointLight
{
   vec4 color;
   vec4 position;
};

layout(std140) uniform CommonBlock
{
   vec3 eye;
   int dummy;
   vec2 screenSize;
   vec2 cubeNearFar;
};

layout(std140) uniform LightsBlock
{
   PointLight pointLight[8];
   int lightCount;
};

uniform sampler2D AO;
uniform sampler2D sampler;
uniform sampler2D normals;

vec3 Shade( const vec3 _Diffuse, const vec3 _LightColor, const vec3 _N, vec3 _L, const vec3 _V, const float _Vis )
{
	// Shading Models for Game Development, Eq. 10
	if (_Vis * dot( _L, _N ) <= 0) return vec3( 0.0 );
	float att = 1.0 / dot( _L, _L );
	_L = normalize( _L );
	vec3 H = normalize( _V + _L );
	float v = 1 - dot( _L, H );
	float fresnel = mix( 0.1, 1, v * v * v * v * v );
	float specularity = 100.0;
	float spec = ((specularity + 2.0) / 8.0) * pow( clamp( dot( H, _N ), 0.0, 1.0 ), specularity ) * fresnel;
	return (dot( _L, _N ) * att * _Vis) * ((spec + 1) * _LightColor * _Diffuse);
}


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
   vec3 Nc = perturb_normal( N, -V, uv );
   float ao = texelFetch( AO, ivec2( gl_FragCoord.xy ), 0 ).r;
   vec3 c = vec3( 0 );
   for( int i = 0; i < lightCount; i++ )
   {
      vec3 L = pointLight[i].position.xyz - wPos;
      float visibility = 1;
      c += Shade( diffuse.rgb, pointLight[i].color.rgb, Nc, L, V, visibility );
   }
   c += vec3( 0.05, 0.05, 0.04 ) * diffuse.rgb;
   c *= ao;
   pixel = vec4( c, (N.y > 0.99) ? 0 : 1 );
}

// EOF
 