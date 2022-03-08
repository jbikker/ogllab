#pragma once

namespace ogllab {

class TextureBuffer;

class Technique
{
public:
	// constructor / destructor
	Technique();
	// methods
	virtual void Init() = 0;
	virtual void Resize( uint _Width, uint _Height ) = 0;
	virtual void Render() = 0;
	RenderTarget* GetScreenBuffer() { return screenBuffer; }
	// data members
protected:
	ScreenBuffer* screenBuffer;
};

class HQTechnique : public Technique
{
public:
	// constructor / destructor
	HQTechnique() {}
	// methods
	void Init();
	void Resize( uint _Width, uint _Height );
	void Render();
	// private methods
	void RenderToCubeMap( const vec3 _Pos );
private:
	// data members
private:
	ShadowCubeTarget* shadowCubeTarget[2];
	CubemapTarget* cubemapTarget;
	GenericTarget* postprocTargetA;
	GenericTarget* postprocTargetB;
	GenericTarget* resizeTarget;
	GenericTarget* saoLinearZTarget;
	GenericTarget* saoRawAOTarget;
	GenericTarget* saoNoisyAOTarget;
	GenericTarget* saoHSmoothTarget;
	GenericTarget* reflectionTarget;
	FXAAPass* fxaa;
	BokehPass* bokeh;
	CubemapPass* cubem;
	ShadowCubePass* shad;
	ForwardPass* fw;
	PrePass* prez;
	CopyPass* resize;
	HQDOFCombine* combine;
	SAOMinifyPass* sao2;
	SAONoisyAOPass* sao3;
	SAOBlurPass* sao4;
	SSRRPass* ssrr;
	FinalizeAOPass* finao;
#ifdef HQDOFEXPERIMENT
	GenericTarget* HQDOFTarget;
	HQDOFPass* hqdof;
#endif
};

class BasicTechnique : public Technique
{
public:
	// constructor / destructor
	BasicTechnique() {}
	// methods
	void Init();
	void Resize( uint _Width, uint _Height );
	void Render();
	// private methods
	void RenderToCubeMap( const vec3 _Pos );
private:
	// data members
private:
	ShadowCubeTarget* shadowCubeTarget[2];
	CubemapTarget* cubemapTarget;
	GenericTarget* postprocTargetA;
	GenericTarget* postprocTargetB;
	FXAAPass* fxaa;
	CubemapPass* cubem;
	ShadowCubePass* shad;
	ForwardPass* fw;
};

class StochasticTechnique : public Technique
{
public:
	// constructor / destructor
	StochasticTechnique() {}
	// methods
	void Init();
	void Resize( uint _Width, uint _Height );
	void Render();
	// data members
private:
	TextureBuffer* textureBuffer;
	GeometryBuffer* gbuffer;
	GenericTarget* postprocTargetA;
	GenericTarget* postprocTargetB;
	GenericTarget* lightTarget;
	GeometryPass* geom;
	StochasticPass* stoc;
	CopyPass* copy;
	FXAAPass* fxaa;
	BlurPass* blur;
	CombinePass* comb;
};

}

// EOF