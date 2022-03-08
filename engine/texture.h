#pragma once

namespace ogllab {

class Texture
{
public:
	// constructor / destructor
	Texture();
	Texture( const char* _File );
	~Texture();
	// get / set
	uint GetID() { return id; }
	void SetID( GLuint _ID ) { id = _ID; }
	void SetName( const char* _Name );
	char* GetName() { return name; }
	bool HasAlpha() { return hasAlpha; }
	void SetAlpha( bool _Alpha ) { hasAlpha = _Alpha; }
	// methods
	void Load( const char* _File );
	void LoadCubemap( const char** _Files );
	void sRGBtoLinear( unsigned char* _Data, uint _Size, uint _Stride );
	// data members
private:
	GLuint id;
	char* name;
	bool hasAlpha;
};

}

// EOF