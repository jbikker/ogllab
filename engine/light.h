#pragma once

namespace ogllab {

class TransformNode;
class LightBase
{
public:
	vec3 color;
	virtual void Bind( const GLuint _ShaderID, const uint _Idx ) = 0;
	TransformNode* node;
};

class PointLight : public LightBase
{
public:
	PointLight( vec3 _Pos, vec3 _Color, TransformNode* _Node ) : position( _Pos ) { color = _Color, node = _Node; }
	void Bind( const GLuint _ShaderID, const uint _Idx );
	vec3 position;
};

class DirectionalLight : public LightBase
{
public:
	DirectionalLight( vec3 _Dir, vec3 _Color ) : direction( _Dir ) { color = _Color, node = 0; }
	void Bind( const GLuint _ShaderID, const uint _Idx );
	vec3 direction;
};

}

// EOF