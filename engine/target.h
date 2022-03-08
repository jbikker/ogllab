#pragma once

namespace ogllab {

class RenderTarget
{
public:
	// get / set
	GLuint GetID() { return fbo; }
	// methods
	virtual void Bind() { /* empty default implementation for cubemap targets */ }
	virtual void Clear() { /* empty default implementation */ }
	void Unbind();
	void Resize( uint _Width, uint _Height );
	float GetAspectRatio() { return (float)width / (float)height; }
	ivec2 GetSize() { return ivec2( width, height ); }
	// data members
protected:
	GLuint fbo;
	uint width, height;
	bool clear;
};

class ShadowCubeTarget : public RenderTarget
{
public:
	ShadowCubeTarget( uint _Width, uint _Height );
	GLuint GetCubemapID() { return cubeMapID; }
	void Bind( int _Face, vec3 _LightPos );
private:
	GLuint cubeMapID;
};

class GeometryBuffer : public RenderTarget
{
public:
	GeometryBuffer( uint _Width, uint _Height );
	GLuint GetBufferID( uint _Idx ) { return buffer[_Idx]; }
	GLuint GetDepthID() { return depth; }
	void Resize( uint _Width, uint _Height );
	void Bind();
private:
	GLuint buffer[4], depth;
};

class CubemapTarget : public RenderTarget
{
public:
	CubemapTarget( uint _Width, uint _Height );
	GLuint GetCubemapID() { return cubeMapID; }
	void Bind( uint _Idx );
private:
	GLuint cubeMapID, depthMapID;
};

class GenericTarget : public RenderTarget
{
public:
	GenericTarget( uint _Width, uint _Height, TargetFlags _Flags = TARGET_DEFAULT );
	GLuint GetColormapID() { return colorID; }
	GLuint GetDepthTextureID() { return depthID; }
	void Resize( uint _Width, uint _Height );
	void Bind();
	void Clear();
private:
	GLuint colorID, depthID;
	GLint colorFormat, depthFormat;
	GLenum colorType, depthType, colorLayout;
	uint flags;
};

class ScreenBuffer : public RenderTarget
{
public:
	ScreenBuffer( uint _Width, uint _Height );
	void Bind();
};

}

// EOF