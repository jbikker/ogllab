#include "precomp.h"

Camera::Camera()
{
	forward = vec3( 0, 0, -1 ); // in OpenGL, -z is into screen, y points up
	position = vec3( 0, 0, 0 );
}

Camera::~Camera()
{
}

vec3 Camera::GetLeft()
{
	vec3 up( 0, 1, 0 );
	return normalize( cross( up, forward ) );
}

vec3 Camera::GetUp()
{
	vec3 left = GetLeft();
	return normalize( cross( forward, left ) );
}

mat4 Camera::GetViewMatrix()
{
	return lookAt( position, position + forward, vec3( 0, 1, 0 ) ); // already inverted
}

// EOF