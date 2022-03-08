#pragma once

namespace ogllab {

#define EPSILON		0.00001f
#define MAXFLOAT	1e33f

class Plane
{
public:
	Plane() {}
	Plane( const vec3& _A, const vec3& _B, const vec3& _C );
	Plane( const vec3& _N, const float _D ) : N( _N ), d( _D ) {}
	void Normalize();	
	float Distance( const vec3& _P );
	vec3 Intersection( const vec3& _A, const vec3& _B );
	vec3 N;
	float d;
};

class AABB
{
public:
	AABB() {}
	vec3 min, max;
};

class Frustum
{
public:
	Frustum() {}
	Frustum( const mat4& _Matrix );
	bool Intersect( const vec3& _P );
	bool Intersect( Frustum& _Candidate );
	bool Intersect( AABB& _Candidate );
	Plane plane[6];
	vec3 vertex[8];
private:
	bool HalfIntersect( Frustum& _Candidate );
};

class Triangle
{
public:
	vec3 v[3];
};

}

// EOF