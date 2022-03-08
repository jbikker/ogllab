#pragma once

namespace ogllab {

class Pass
{
public:
	// I/O definition
	struct DefaultInputs
	{
		GLuint color;
		GLuint depth;
	};
	// get / set
	void SetTarget( RenderTarget* _Target ) { target = _Target; }
	static void SetScene( Scene* _Scene ) { scene = _Scene; }
	// methods
	void PrepareExecute();
	void AttachTexture( uint _Idx, const char* _Name, GLuint _ID, bool _Depth = false );
	virtual void Execute() = 0;
	// data members
protected:
	RenderTarget* target;
	Shader* shader;
	static Scene* scene;
};

class DebugPass : public Pass
{
public:
	// I/O definition
	struct DebugInputs 
	{ 
		GLuint data;	
	};
	// constructor / destructor
	DebugPass();
	// methods
	void Execute();
	// data members
public:
	DebugInputs inputs;
};

class PrePass : public Pass
{
public:
	// constructor / destructor
	PrePass();
	// methods
	void Execute();
};

class FXAAPass : public Pass
{
public:
	// constructor / destructor
	FXAAPass();
	// methods
	void Execute();
	// data members
public:
	DefaultInputs inputs;
};

class BokehPass : public Pass
{
public:
	// I/O definition
	struct DefaultInputs
	{
		GLuint color;
		GLuint depth;
		float focalDepth;
		float focalLength;
		float fstops;
	};
	// constructor / destructor
	BokehPass();
	// methods
	void Execute();
	// data members
	DefaultInputs inputs;
};

class HQDOFPass : public Pass
{
public:
	// I/O definition
	struct DOFInputs
	{
		GLuint color;
		GLuint depth;
		ivec2 resolution;
	};
	// constructor / destructor
	HQDOFPass();
	// methods
	void Execute();
	// data members
	DOFInputs inputs;
	Mesh* DOFGrid;
	ivec2 resolution;
	Texture* bokeh;
};

class HQDOFCombine : public Pass
{
public:
	// I/O definition
	struct CombineInputs
	{
		GLuint color;
		GLuint depth;
		GLuint dof;
	};
	// constructor / destructor
	HQDOFCombine();
	// methods
	void Execute();
	// data members
	CombineInputs inputs;
};

class BlurPass : public Pass
{
public:
	// I/O definition
	struct BlurInputs
	{
		GLuint color;
		GLuint key;
		uint axis; // 0 = x, 1 = y
	};
	// constructor / destructor
	BlurPass();
	// methods
	void Execute();
	// data members
	BlurInputs inputs;
};

class CopyPass : public Pass
{
public:
	// I/O definition
	struct CopyInputs
	{
		GLuint color;
	};
	// constructor / destructor
	CopyPass();
	// methods
	void Execute();
	// data members
	CopyInputs inputs;
};

class CubemapPass : public Pass
{
public:
	// I/O definition
	struct CubemapInputs
	{
		vec3 position;
	};
	// constructor / destructor
	CubemapPass() { /* nothing to do */ }
	// methods
	void Execute();
	// data members
	CubemapInputs inputs;
};

class ShadowCubePass : public Pass
{
public:
	// I/O definition
	struct ShadowCubeInputs
	{
		vec3 position;
		vec2 nearFar;
		mat4 viewMatrix;
	};
	// constructor / destructor
	ShadowCubePass();
	// methods
	void Execute();
	// data members
	ShadowCubeInputs inputs;
};

class ShadowMapPass : public Pass
{
public:
	// I/O definition
	struct ShadowMapInputs
	{
		float size;
		vec3 direction;
	};
	// constructor / destructor
	ShadowMapPass();
	// methods
	void Execute();
	// data members
	ShadowMapInputs inputs;
};

class SAOMinifyPass : public Pass
{
public:
	// I/O definition
	struct MinifyInputs
	{
		GLuint data;
	};
	// constructor / destructor
	SAOMinifyPass();
	// methods
	void Execute();
	// data members
	MinifyInputs inputs;
};

class SAONoisyAOPass : public Pass
{
public:
	// I/O definition
	struct AOInputs
	{
		GLuint depth;
		float scale;
		float radius;
		float bias;
		float intensity;
	};
	// constructor / destructor
	SAONoisyAOPass();
	// methods
	void Execute();
	// data members
	AOInputs inputs;
};

class SAOBlurPass : public Pass
{
public:
	// I/O definition
	struct HBlurInputs
	{
		GLuint data;
		uint axis; // 0 = x, 1 = y
	};
	// constructor / destructor
	SAOBlurPass();
	// methods
	void Execute();
	// data members
	HBlurInputs inputs;
};

class SSRRPass : public Pass
{
public:
	// constructor / destructor
	SSRRPass();
	// methods
	void Execute();
	// data members
	DefaultInputs inputs;
};

class ForwardPass : public Pass
{
public:
	// I/O definition
	struct ForwardInputs
	{
		mat4 viewMatrix;
	};
	// constructor / destructor
	ForwardPass() {}
	// methods
	void Execute();
	// data members
	ForwardInputs inputs;
};

class GeometryPass : public Pass
{
public:
	// I/O definition
	struct GeometryInputs
	{
		mat4 viewMatrix;
	};
	// constructor / destructor
	GeometryPass() {}
	// methods
	void Execute();
	// data members
	GeometryInputs inputs;
};

class StochasticPass : public Pass
{
public:
	// I/O definition
	struct StochasticInputs
	{
		GLuint wpos;
		GLuint color;
		GLuint normal;
	};
	// constructor / destructor
	StochasticPass();
	// methods
	void Execute();
	// data members
	StochasticInputs inputs;
};

class CombinePass : public Pass
{
public:
	// I/O definition
	struct CombineInputs
	{
		GLuint diffuse;
		GLuint light;
	};
	// constructor / destructor
	CombinePass();
	// methods
	void Execute();
	// data members
	CombineInputs inputs;
};

class FinalizeAOPass : public Pass
{
public:
	// constructor / destructor
	FinalizeAOPass();
	// methods
	void Execute();
	// data members
	DefaultInputs inputs;
};

}

// EOF