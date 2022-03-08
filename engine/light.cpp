#include "precomp.h"

void PointLight::Bind( const GLuint _ShaderID, const uint _Idx )
{
	char colorName[64], posName[64];
	sprintf( colorName, "pointLight[%i].color", _Idx );
	sprintf( posName, "pointLight[%i].position", _Idx );
	int i = glGetUniformLocation( _ShaderID, colorName );
	int j = glGetUniformLocation( _ShaderID, posName );
	glUniform3f( glGetUniformLocation( _ShaderID, colorName ), color.x, color.y, color.z );
	glUniform3f( glGetUniformLocation( _ShaderID, posName ), position.x, position.y, position.z );
}

void DirectionalLight::Bind( const GLuint _ShaderID, const uint _Idx )
{
}

// EOF