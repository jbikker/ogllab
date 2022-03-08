#include "precomp.h"

Plane::Plane( const vec3& _A, const vec3& _B, const vec3& _C )
{
	N = cross( normalize( _B - _A ), normalize( _C - _A ) );
	d = dot( N, _A );
}

void Plane::Normalize()
{
	const float r = 1.0f / length( N );
	N *= r, d *= r;
}

float Plane::Distance( const vec3& _P )
{
	return dot( N, _P ) - d;
}

vec3 Plane::Intersection( const vec3& _A, const vec3& _B )
{
	const vec3 D = _B - _A;
	const float t = (-dot( _A, N ) + d) / dot( D, N );
	return _A + t * D;
}

Frustum::Frustum( const mat4& _Matrix )
{
	// determine vertices
	const mat4 inv = inverse( _Matrix );
	vec4 v[8] = { inv * vec4( -1, -1, -1, 1 ), inv * vec4(  1, -1, -1, 1 ),
				  inv * vec4(  1,  1, -1, 1 ), inv * vec4( -1,  1, -1, 1 ),
				  inv * vec4( -1, -1,  1, 1 ), inv * vec4(  1, -1,  1, 1 ),
				  inv * vec4(  1,  1,  1, 1 ), inv * vec4( -1,  1,  1, 1 ) };
	// scale
	for( int i = 0; i < 8; i++ ) 
	{
		const float r = 1.0f / v[i].w;
		vertex[i] = vec3( v[i].x * r, v[i].y * r, v[i].z * r );
	}
	// determine planes
	plane[0] = Plane( vertex[4], vertex[7], vertex[0] );
	plane[1] = Plane( vertex[5], vertex[4], vertex[1] );
	plane[2] = Plane( vertex[6], vertex[5], vertex[2] );
	plane[3] = Plane( vertex[7], vertex[6], vertex[3] );
	plane[4] = Plane( vertex[1], vertex[0], vertex[3] );
	plane[5] = Plane( vertex[4], vertex[5], vertex[7] );
	for ( int i = 0; i < 6; i++ ) plane[i].Normalize();
}

bool Frustum::Intersect( const vec3& _P )
{
	for( int p = 0; p < 6; p++ ) if ((dot( _P, plane[p].N ) - plane[p].d) < -EPSILON) return false;
	return true;
}

bool Frustum::HalfIntersect( Frustum& _Candidate )
{
	// see if any of our points are in _Candidate
	for( int v = 0; v < 8; v++ ) if (Intersect( _Candidate.vertex[v] )) return true;
	// see if any our our edges intersect _Candidate
	uint e0[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3 }, e1[12] = { 1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7 };
	for( int i = 0; i < 12; i++ )
	{
		const vec3 A = _Candidate.vertex[e0[i]], B = _Candidate.vertex[e1[i]];
		for( int p = 0; p < 6; p++ )
		{
			const float d1 = plane[p].Distance( A ), d2 = plane[p].Distance( B );
			if (((d1 < 0) && (d2 > 0)) || ((d1 > 0) && (d2 < 0)))
			{
				const vec3 P = plane[p].Intersection( A, B );
				if (Intersect( P )) return true;
			}
		}
	}
	return false;
}

bool Frustum::Intersect( Frustum& _Candidate )
{
	return (HalfIntersect( _Candidate ) || _Candidate.HalfIntersect( *this ));
}

/*
bool Frustum::Intersect( AABB& _Candidate )
{
	// Returns: INTERSECT : 0  INSIDE : 1   OUTSIDE : 2 
	// int FrustumAABBIntersect(Plane *planes, Vector &mins, Vector &maxs) { 
	int ret = INSIDE; 
	vec3 vmin, vmax; 
	for( uint i = 0; i < 6; ++i ) 
	{ 
		if (plane[i].N.x > 0) vmin.x = mins.x, vmax.x = maxs.x; 
						 else vmin.x = maxs.x, vmax.x = mins.x; 
		if (plane[i].N.y > 0) vmin.y = mins.y, vmax.y = maxs.y; 
						 else vmin.y = maxs.y, vmax.y = mins.y; 
		if (plane[i].N.z > 0) vmin.z = mins.z, vmax.z = maxs.z; 
						 else vmin.z = maxs.z, vmax.z = mins.z; 
		if (dot( plane[i].N, vmin ) + plane[i].d > 0) return OUTSIDE; 
		if (dot( plane[i].N, vmax ) + plane[i].d >= 0) ret = INTERSECT; 
	} 
	return ret;
}
*/

bool Frustum::Intersect( AABB& _Candidate ) // from zach.in.tu-clausthal.de
{
	vec3 vmin, vmax, &mins = _Candidate.min, &maxs = _Candidate.max; 
	for( uint i = 0; i < 6; ++i ) 
	{ 
		if (plane[i].N.x < 0) vmin.x = mins.x, vmax.x = maxs.x; 
						 else vmin.x = maxs.x, vmax.x = mins.x; 
		if (plane[i].N.y < 0) vmin.y = mins.y, vmax.y = maxs.y; 
						 else vmin.y = maxs.y, vmax.y = mins.y; 
		if (plane[i].N.z < 0) vmin.z = mins.z, vmax.z = maxs.z; 
						 else vmin.z = maxs.z, vmax.z = mins.z; 
		if ((dot( plane[i].N, vmin ) - plane[i].d) < 0) return false;
	} 
	return true;
}

// EOF