#version 330
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2D color;
uniform sampler2D depth;

uniform vec2 screenSize;
uniform float FXAA_SPAN_MAX = 8.0;
uniform float FXAA_REDUCE_MUL = 1.0 / 8.0;

in vec2 uv;

out vec3 pixel;

vec3 FxaaPixelShader( vec4 posPos, sampler2D tex, vec2 rcpFrame )
{   
	#define FxaaTexLod0(t, p) textureLod( t, p, 0.0 )
    #define FXAA_REDUCE_MIN (1.0 / 128.0)
    vec3 rgbNW = FxaaTexLod0( tex, posPos.zw ).xyz;
    vec3 rgbNE = texture2DLodOffset( tex, posPos.zw, 0, ivec2( 1, 0 ) ).xyz;
    vec3 rgbSW = texture2DLodOffset( tex, posPos.zw, 0, ivec2( 0, 1 ) ).xyz;
    vec3 rgbSE = texture2DLodOffset( tex, posPos.zw, 0, ivec2( 1, 1 ) ).xyz;
    vec3 rgbM  = FxaaTexLod0( tex, posPos.xy ).xyz;
    vec3 luma = vec3( 0.299, 0.587, 0.114 );
    float lumaNW = dot( rgbNW, luma );
    float lumaNE = dot( rgbNE, luma );
    float lumaSW = dot( rgbSW, luma );
    float lumaSE = dot( rgbSE, luma );
    float lumaM  = dot( rgbM,  luma );
    float lumaMin = min( lumaM, min( min( lumaNW, lumaNE ), min( lumaSW, lumaSE ) ) );
    float lumaMax = max( lumaM, max( max( lumaNW, lumaNE ), max( lumaSW, lumaSE ) ) );
    vec2 dir; 
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    float dirReduce = max( (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN );
    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min( vec2(  FXAA_SPAN_MAX,  FXAA_SPAN_MAX ), max( vec2( -FXAA_SPAN_MAX, -FXAA_SPAN_MAX ), dir * rcpDirMin)) * rcpFrame.xy;
    vec3 rgbA = (1.0/2.0) * (FxaaTexLod0( tex, posPos.xy + dir * (1.0 / 3.0 - 0.5)).xyz + FxaaTexLod0( tex, posPos.xy + dir * (2.0 / 3.0 - 0.5)).xyz );
    vec3 rgbB = rgbA * (1.0 / 2.0) + (1.0 / 4.0) * (FxaaTexLod0( tex, posPos.xy + dir * (0.0 / 3.0 - 0.5)).xyz + FxaaTexLod0( tex, posPos.xy + dir * (3.0 / 3.0 - 0.5)).xyz );
    float lumaB = dot( rgbB, luma );
    if ((lumaB < lumaMin) || (lumaB > lumaMax)) return rgbA;
    return rgbB; 
}

vec3 PostFX( sampler2D tex, vec2 uv )
{
	vec4 posPos;
	posPos.xy = uv;
	vec2 rcpFrame = vec2( 1.0 / screenSize.x, 1.0 / screenSize.y );
	posPos.zw = uv - rcpFrame * (0.5 + 1.0 / 4.0);
	return FxaaPixelShader( posPos, tex, rcpFrame );
}

float reconstructCSZ( float d ) 
{
	float znear = -0.1; 
	float zfar = -500;
	float ci0 = znear * zfar;
	float ci1 = znear - zfar;
	float ci2 = zfar;
	return ci0 / (ci1 * d + ci2);
}

void main()
{
	vec2 uv = gl_FragCoord.xy / screenSize;
	float d = 1 - length( uv - vec2( 0.5, 0.5 ) );
	d = min( 1.0, d * 1.8 );
	vec3 linear = d * PostFX( color, uv ) * 2.5;
	pixel = vec3( sqrt( linear.r ), sqrt( linear.g ), sqrt( linear.b ) );
}

// EOF