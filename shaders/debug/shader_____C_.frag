#version 330 core

in vec3 N;
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

uniform vec3 color;
uniform sampler2D AO;
uniform samplerCubeShadow shadowCube[2];

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


void main()
{
   vec4 diffuse = vec4( color, 1 );
   vec3 V = normalize( eye - wPos );
   vec3 Nc = normalize( N );
   float ao = texelFetch( AO, ivec2( gl_FragCoord.xy ), 0 ).r;
   vec3 c = vec3( 0 );
   for( int i = 0; i < lightCount; i++ )
   {
      vec3 L = pointLight[i].position.xyz - wPos;
      float visibility = 1;
      if (i < 2)
      {
         float fragDepth = length( L );
         float bias = 0.03 * tan( acos( dot( N, normalize( L ) ) ) );
         visibility = texture( shadowCube[i], vec4( -L, (fragDepth - bias) / cubeNearFar.y ) );
      }
      c += Shade( diffuse.rgb, pointLight[i].color.rgb, Nc, L, V, visibility );
   }
   c += vec3( 0.05, 0.05, 0.04 ) * diffuse.rgb;
   c *= ao;
   pixel = vec4( c, (N.y > 0.99) ? 0 : 1 );
}

// EOF
 