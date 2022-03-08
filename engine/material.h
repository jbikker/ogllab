#pragma once

namespace ogllab {

class Material
{
public:
	// constructor / destructor
	Material();
	~Material();
	// get / set
	void SetName( const char* _Name );
	char* GetName() { return name; }
	void SetColor( vec3 _Color ) { color = _Color; }
	vec3& GetColor() { return color; }
	void SetShader( RenderType _Type, Shader* _Shader ) { shader[_Type] = _Shader; }
	Shader* GetShader( RenderType _Type ) { return shader[_Type]; }
	Shader* GetActiveShader() { return activeShader; }
	void SetTexture( Texture* _Texture ) { texture = _Texture; }
	Texture* GetTexture() { return texture; }
	void SetNormalMap( Texture* _Normals ) { normalMap = _Normals; }
	Texture* GetNormalMap() { return normalMap; }
	void SetEnvMap( Texture* _Env ) { envMap = _Env; }
	Texture* GetEnvMap() { return envMap; }
	// methods
	void Init( char* _Texture, char* _NormalMap = 0 );
	void Init( char* _Texture, Shader* _Shader );
	void CreateShaders();
	void Bind( RenderType _Type );
	void Unbind();
	// data members
private:
	Shader* shader[4], *activeShader;
	GLuint colorBuffer;
	Texture* texture;
	Texture* normalMap;
	Texture* envMap;
	vec3 color;
	char* name;
public:
	static GLuint currentShader;
};

}

// EOF