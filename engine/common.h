#pragma once

#define DEFAULT_SCREENWIDTH		1280
#define DEFAULT_SCREENHEIGHT	 800

#define PI	3.14159265358979323846264f
#define GLM_FORCE_RADIANS

#define MAXLIGHTS				8
#define MAXDYNAMIC				2

#define HQTECHNIQUE				1
#define BASICTECHNIQUE			2
#define STOCHASTICTECHNIQUE		3

#define TECHNIQUE				1		// techique to use

// #define HQDOFEXPERIMENT				// enable high quality dof code
// #define SORTEDRENDERING					// sorted rendering experiment
// #define SKIPDOF						// disable depth of field
#define NOALPHA							// disable alpha
// #define UNFILTEREDAO					// raw ao result
// #define JUSTAO							// only show ao result

enum RenderType
{	RENDER_DIRECT = 0,
	RENDER_SHADOW = 1,
	RENDER_SHADOWCUBE = 2,
	RENDER_GEOMETRY = 3,
	RENDER_SKY = 4
};

enum TargetFlags
{
	TARGET_DEPTH = 1,
	TARGET_COLOR = 2,
	TARGET_FLOATCOLOR = 6,
	TARGET_DEFAULT = 7,					// includes float, color, depth
	TARGET_DEPTH16 = 8,
	TARGET_1CHANNEL = 22,				// includes float, colo
	TARGET_MIPMAP = 32,
	TARGET_MIPCOLOR = 38,				// includes color
	TARGET_MIPCOLZ = 39					// includes float, color, depth, mip
};

enum NodeFlags
{
	NODE_STATIC = 1,
	NODE_DYNAMIC = 2
};

typedef unsigned int ShaderFlags;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define MALLOC64(x) _aligned_malloc(x,64)
#define FREE64(x) _aligned_free(x)

enum
{
	GEOMETRY = 1,
	SHADOW = 2,
	SHADOWCUBE = 4,
	TEXTURE = 8,
	NORMALMAP = 16,
	ENVMAP = 32,
	ALPHA = 64,
};

#define ENABLEAVX										// enable to use AVX (i7 and later)

// #define SHADOWS										// enable for ray traced shadows

#define MAPSIZE			20								// spawn area, fetched before first frame

#define CHUNKSIZE		128								// octree size
#define CHUNKBITS		7								// log2( CHUNKSIZE )

#define GLOBALSCALE		1.2f							// landscape scale value
#define INVGSCALE		(1.0f / GLOBALSCALE)			// reciprocal of landscape scale

#define WORLDXZ			64								// octree grid size (x,z)
#define WORLDY			4								// octree grid height

#define PACKETWH		16								// ray packet width (optimal for i7)
#define PACKETSIZE		(PACKETWH * PACKETWH)			// ray packet size
#define PACKETQ			(PACKETSIZE / 4)				// __m128 per ray packet value
#define PACKETO			(PACKETSIZE / 8)				// __m256 per ray packet value
#define METATILE		2								
#define METATILES		(METATILE * METATILE)			// packets traversing grid simultaneously

#define CORNER1IDX		(0)
#define CORNER2IDX		(PACKETWH * 2 - 3)
#define CORNER3IDX		((PACKETWH/2)*((PACKETWH/2)-1)*4+2)
#define CORNER4IDX		(PACKETSIZE-1)

struct timer 
{ 
	typedef long long value_type; 
	static double inv_freq; 
	value_type start; 
	timer() : start( get() ) { init(); } 
	float elapsed() const { return (float)((get() - start) * inv_freq); } 
	static value_type get() 
	{ 
		LARGE_INTEGER c; 
		QueryPerformanceCounter( &c ); 
		return c.QuadPart; 
	} 
	static double to_time(const value_type vt) { return double(vt) * inv_freq; } 
	void reset() { start = get(); }
	static void init() 
	{ 
		LARGE_INTEGER f; 
		QueryPerformanceFrequency( &f ); 
		inv_freq = 1000./double(f.QuadPart); 
	} 
}; 

// EOF