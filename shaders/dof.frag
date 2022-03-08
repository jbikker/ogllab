#version 330

uniform sampler2D color;
uniform sampler2D depth;
uniform vec2 screenSize;

in vec2 uv;
out vec3 pixel;

#define PI  3.14159265

uniform float focalDepth;			// focal distance value in meters
uniform float focalLength;			// focal length in mm
uniform float fstop;				// f-stop value

float znear = 0.1;					// camera clipping start
float zfar = 500.0;					// camera clipping end
int samples = 5;					// samples on the first ring
int rings = 4;						// ring count
float CoC = 0.03;					// circle of confusion size in mm (35mm film = 0.03mm)
bool vignetting = true;				// use optical lens vignetting?
float vignout = 1.3;				// vignetting outer border
float vignin = 0.0;					// vignetting inner border
float vignfade = 22.0;				// f-stops till vignete fades
bool autofocus = true;				// use autofocus in shader
vec2 focus = vec2( 0.5, 0.5 );		// autofocus point on screen
float maxblur = 1.5;				// clamp value of max blur
float threshold = 2.5;				// highlight threshold;
float gain = 2.0;					// highlight gain;
float bias = 0.5;					// bokeh edge bias
float fringe = 0.7;					// bokeh chromatic aberration/fringing
bool noise = true;					// use noise instead of pattern for sample dithering
float namount = 0.0002;				// dither amount
float dbsize = 1.25;				// depthblursize

float width = screenSize.x;
float height = screenSize.y;
vec2 texel = vec2( 1.0 / 1280.0, 1.0 / 800.0 );

vec3 getcolor( vec2 coords, float blur ) // processing the sample
{
	vec3 col = vec3( 0.0 );
	col.r = texture2D( color, coords + vec2( 0.0, 1.0 ) * texel * fringe * blur ).r;
	col.g = texture2D( color, coords + vec2( -0.866, -0.5 ) * texel * fringe * blur ).g;
	col.b = texture2D( color, coords + vec2( 0.866, -0.5 ) * texel * fringe * blur ).b;
	float lum = dot( col.rgb, vec3( 0.299, 0.587, 0.114 ) );
	float thresh = max( (lum - threshold) * gain, 0.0 );
	return col + mix( vec3( 0.0 ), col, thresh * blur );
}

void main() 
{
	float d_ = texture2D( depth, uv ).r, fDepth = focalDepth;
	if (autofocus) fDepth = texture2D( depth, focus ).x;
	float blur = 0.0;
	{
		float f = focalLength;			// focal length in mm
		float d = fDepth * 1000.0;		// focal plane in mm
		float o = d_ * 1000.0;			// depth in mm
		float a = (o * f) / (o - f); 
		float b = (d * f) / (d - f); 
		float c = (d - f) / (d * fstop * CoC); 
		blur = abs( a - b ) * c;
	}
	blur = clamp( blur, 0.0, 1.0 );
	float w = texel.x * blur * maxblur;
	float h = texel.y * blur * maxblur;
	if (blur < 0.05) pixel = texture2D( color, uv ).rgb; else
	{
		vec3 col = texture2D( color, uv ).rgb;
		// int rings = int( blur * 2.0 );
		float s = 1.0, fi = 1.4, ir = 1.0f / float( rings );
		int ringsamples = samples;
		for( int i = 1; i <= rings; ++i, fi += 1.4, ringsamples += 4 )
		{   
			float fj = 0, step = PI * 2.0 / float( ringsamples );
			float a = mix( 1.0, fi * ir, bias );
			for( int j = 0; j < ringsamples; ++j, fj += step, s += a )   
			{
				float pw = cos( fj ) * fi;
				float ph = sin( fj ) * fi;
			#if 0
				col += getcolor( uv + vec2( pw * w, ph * h ), blur ) * a;  
			#else
				col += texture( color, uv + vec2( pw * w, ph * h ), blur ).rgb * a;
			#endif
			}
		}
		pixel = col / s;
	}
}

// DoF with bokeh GLSL shader v2.4 - martinsh - devlog-martinsh.blogspot.com
// licensed under a Creative Commons Attribution 3.0 Unported License.

// EOF