#pragma once

namespace ogllab {

class Shader 
{
public:
	// constructor / destructor
	Shader( const char* _VFile, const char* _PFile );
	Shader( const char* _GFile, const char* _VFile, const char* _PFile );
	Shader( ShaderFlags _Flags );
	~Shader();
	// get / set
	uint GetID();
	char* GetName() { return name; }
	void SetName( const char* _Name );
	// methods
	void Init( const char* _VFile, const char* _PFile, const char* _GFile = 0 );
	void Compile( const char* _Vertex, const char* _Fragment, const char* _Geometry = 0 );
	void Bind();
	void Unbind();
	// static methods
	static Shader* GetCached( ShaderFlags _Flags );
	static void ConstructName( ShaderFlags _Flags, char* _Name );
	static void IncludeCode( string& _Code, const char* _File );
	// data members
private:
	uint id;		// shader program identifier
	uint geometry;	// geometry shader identifier
	uint vertex;	// vertex shader identifier
	uint pixel;		// fragment shader identifier
	char* name;		// name in cache
	static vector<Shader*> cache;
};

}

// EOF