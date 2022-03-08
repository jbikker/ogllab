#pragma once

namespace ogllab {

class Camera
{
public:
	// constructor / destructor
	Camera();
	~Camera();
	// get / set
	mat4 GetViewMatrix();
	vec3 GetPosition() { return position; }
	void SetPosition( const vec3& _Position ) { position = _Position; }
	void SetForward( const vec3& _Forward ) { forward = _Forward; }
	vec3 GetForward() { return forward; }
	vec3 GetLeft();
	vec3 GetUp();
	// data members
private:
	vec3 forward, position;
};

}

// EOF