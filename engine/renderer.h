#pragma once

namespace ogllab {

class DrawCall
{
public:
	uint key;			// sort key
	uint transform;		// index into separate vector of transforms
	MeshBase* mesh;		// contains vao, material, tris
};

class DrawTransform
{
public:
	mat4 world, view, light;
};

class UniformBuffer
{
public:
	// constructor / destructor
	UniformBuffer( void* _Data, uint _Size, uint _Point );
	// get / set
	GLuint GetID() { return id; }
	// methods
	void Update();
	// data members
private:
	float* data;
	uint size, bindingPoint;
	GLuint id;
};

class TextureBuffer
{
public:
	// constructor / destructor
	TextureBuffer( uint _Size );
	// get / set
	GLuint GetBufferID() { return tbo; }
	GLuint GetTextureID() { return tex; }
	float* GetData() { return data; }
	// methods
	void Update();
	// data members
private:
	float* data;
	uint size;
	GLuint tbo, tex;
};

struct StaticData
{
	struct _PLight { vec3 col; float d1; vec3 pos; float d2; };
	// constant frame data
	GLuint shadowMapID[MAXDYNAMIC];	// id of the textures used in the shadow passes
	GLuint shadowShaderID;			// id of the shader used in the shadow pass
	UniformBuffer* lightUBO;		// lightUniforms buffer object
	UniformBuffer* transformUBO;	// transformUniforms buffer object
	UniformBuffer* commonUBO;		// global texture IDs buffer object
	TextureBuffer* lightBuffer;		// light data for stochastic technique
	// uniforms available to all shaders
	union
	{
		float commonUniforms[8];
		struct
		{
			vec3 eye;
			int dummy;
			float screenWidth, screenHeight;
			float cubeNear, cubeFar;
		};
	};
	union
	{
		float lightUniforms[8 * MAXLIGHTS + 1];
		struct
		{
			_PLight light[MAXLIGHTS];
			int lightCount;
		};
	};
	union
	{
		float transformUniforms[2 * 16];
		struct 
		{ 
			mat4 view;
			mat4 world;
		};
	};
};

class Renderer 
{
public:
	// constructor / destructor
	Renderer();
	~Renderer();
	// methods
	void Init( HWND _Handle );
	bool Create30Context();
	void SetupScene();
	void ReshapeWindow( uint w, uint h );
	void Render();
	void KeyDown( uint _Key ) { key[_Key] = true; }
	void KeyUp( uint _Key ) { key[_Key] = false; }
	// private methods
private:
	void RenderDirect( bool _Shadows );
	void RenderWidthShadows();
	// data members
private:
	Technique* technique;
protected:
	HGLRC hrc;
	HDC hdc;
	HWND hwnd;
	bool* key;
public:
	inline static StaticData statics;
	inline static mat4 viewMatrix, projMatrix;
	inline static GLuint AOdata;
	inline static Scene* scene;
	inline static Camera* camera;
	inline static Frustum* frustum;
	inline static vector<DrawCall> drawCall;
	inline static vector<DrawTransform> drawTransform;
};

}

// EOF