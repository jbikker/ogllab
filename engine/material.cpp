#include "precomp.h"

GLuint Material::currentShader = 0;

Material::Material() : texture( 0 ), normalMap( 0 ), envMap( 0 ), name( 0 ), activeShader( 0 ), colorBuffer( 0 )
{
	memset( shader, 0, sizeof( shader ) );
}

Material::~Material()
{
	for( int i = 0; i < 3; i++ ) delete shader[i];
	delete texture;
	delete normalMap;
	delete name;
}

void Material::SetName( const char* _Name )
{
	delete name;
	name = new char[strlen( _Name ) + 1];
	strcpy( name, _Name );
}

void Material::Init( char* _Texture, char* _NormalMap )
{
	if (_Texture)
	{
		texture = new Texture();
		texture->Load( _Texture );
	}
	if (_NormalMap)
	{
		normalMap = new Texture();
		normalMap->Load( _NormalMap );
	}
	CreateShaders();
}

void Material::Init( char* _Texture, Shader* _Shader )
{
	if (_Texture)
	{
		texture = new Texture();
		texture->Load( _Texture );
	}
	shader[RENDER_DIRECT] = _Shader;
}

void Material::CreateShaders()
{
	ShaderFlags flags = (texture ? TEXTURE : 0) + (normalMap ? NORMALMAP : 0) + (envMap ? ENVMAP : 0);
	if (texture) if (texture->HasAlpha()) flags |= ALPHA;
	Shader* directShader = Shader::GetCached( flags );
	Shader* shadowShader = Shader::GetCached( flags | SHADOW );
	Shader* shadowCubeShader = Shader::GetCached( flags | SHADOWCUBE );
	Shader* geometryShader = Shader::GetCached( flags | GEOMETRY );
	shader[RENDER_DIRECT] = directShader ? directShader : new Shader( flags );
	shader[RENDER_SHADOW] = shadowShader ? shadowShader : new Shader( flags | SHADOW );
	shader[RENDER_SHADOWCUBE] = shadowCubeShader ? shadowCubeShader : new Shader( flags | SHADOWCUBE );
	shader[RENDER_GEOMETRY] = geometryShader ? geometryShader : new Shader( flags | GEOMETRY );
	CheckGL();
}

void Material::Bind( RenderType _Type )
{
	// bind shader, if different from previous material
	uint shaderIndex = (_Type == RENDER_SKY) ? RENDER_DIRECT : _Type;
	if (shader[shaderIndex]->GetID() != currentShader)
	{
		activeShader = shader[shaderIndex];
		activeShader->Bind();
	}
	// bind textures
	uint idx = 0;
	if (_Type != RENDER_GEOMETRY)
	{
		// ambient occlusion
		if (_Type != RENDER_SKY)
		{
			glActiveTexture( GL_TEXTURE0 + idx++ );
			glBindTexture( GL_TEXTURE_2D, Renderer::AOdata );
		}
		// cubemap shadows
		if (_Type == RENDER_SHADOWCUBE)
		{
			for( int i = 0; i < MAXDYNAMIC; i++ )
			{
				glActiveTexture( GL_TEXTURE0 + idx++ );
				glBindTexture( GL_TEXTURE_CUBE_MAP, Renderer::statics.shadowMapID[i] );
			}
		}
		// shadow map
		if (_Type == RENDER_SHADOW)
		{
			glActiveTexture( GL_TEXTURE0 + idx++ );
			glBindTexture( GL_TEXTURE_2D, Renderer::statics.shadowMapID[0] );
		}
	}
	// material texture
	if (texture)
	{
		glActiveTexture( GL_TEXTURE0 + idx++ );
		glBindTexture( GL_TEXTURE_2D, texture->GetID() );
	}
	else
	{
		// no texture; set diffuse color
		if (!texture) 
		{
			GLuint diffuseID = glGetUniformLocation( activeShader->GetID(), "color" );
			glUniform3f( diffuseID, color.x, color.y, color.z );
		}
	}
	// material normalmap
	if (normalMap)
	{
		glActiveTexture( GL_TEXTURE0 + idx++ );
		glBindTexture( GL_TEXTURE_2D, normalMap->GetID() );
	}
	// environment map
	if (_Type != RENDER_GEOMETRY) if (envMap)
	{
		glActiveTexture( GL_TEXTURE0 + idx++ );
		glBindTexture( GL_TEXTURE_CUBE_MAP, envMap->GetID() );
	}
	CheckGL();
}

void Material::Unbind()
{
	activeShader->Unbind();
	activeShader = 0;
}

// EOF