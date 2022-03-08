#include "precomp.h"

static bool firstFrame = true;

Technique::Technique()
{
	// screen target
	screenBuffer = new ScreenBuffer( 64,64 );
}

void HQTechnique::Init()
{
	// targets
	postprocTargetA = new GenericTarget( 64, 64 );
	postprocTargetB = new GenericTarget( 64, 64 );
	shadowCubeTarget[0] = new ShadowCubeTarget( 2048, 2048 );
	shadowCubeTarget[1] = new ShadowCubeTarget( 2048, 2048 );
	cubemapTarget = new CubemapTarget( 1024, 1024 );
	resizeTarget = new GenericTarget( 64, 64, TARGET_FLOATCOLOR );
	saoLinearZTarget = new GenericTarget( 64, 64, TARGET_MIPCOLZ );
	saoRawAOTarget = new GenericTarget( 64, 64, TARGET_COLOR );
	saoNoisyAOTarget = new GenericTarget( 64, 64, TARGET_COLOR );
	saoHSmoothTarget = new GenericTarget( 64, 64, TARGET_COLOR );
	reflectionTarget = new GenericTarget( 64, 64, TARGET_FLOATCOLOR );
	// render passes
	fxaa = new FXAAPass();
	bokeh = new BokehPass();
	cubem = new CubemapPass();
	shad = new ShadowCubePass();
	fw = new ForwardPass();
	prez = new PrePass();
	combine = new HQDOFCombine();
	resize = new CopyPass();
	sao2 = new SAOMinifyPass();
	sao3 = new SAONoisyAOPass();
	sao4 = new SAOBlurPass();
	ssrr = new SSRRPass();
	finao = new FinalizeAOPass();
#ifdef HQDOFEXPERIMENT
	// hq dof test code
	HQDOFTarget =  new GenericTarget( 128, 64, TARGET_FLOATCOLOR );
	hqdof = new HQDOFPass();
#endif
}

void HQTechnique::Resize( uint _Width, uint _Height )
{
	screenBuffer->Resize( _Width, _Height );
	postprocTargetA->Resize( _Width, _Height );
	postprocTargetB->Resize( _Width, _Height );
	saoLinearZTarget->Resize( _Width, _Height );
	saoNoisyAOTarget->Resize( _Width, _Height );
	saoHSmoothTarget->Resize( _Width, _Height );
	saoRawAOTarget->Resize( _Width, _Height );
	resizeTarget->Resize( _Width >> 1, _Height >> 1 );
	reflectionTarget->Resize( _Width, _Height );
#ifdef HQDOFEXPERIMENT
	HQDOFTarget->Resize( _Width, _Height >> 1 ); // half res, two panes
#endif
}

void HQTechnique::RenderToCubeMap( const vec3 _Pos )
{
	cubem->SetTarget( cubemapTarget );
	cubem->inputs.position = _Pos;
	cubem->Execute();
	// assign cubemap to specific scene material
	Material* domeMat = Renderer::scene->FindMaterial( "vehicle_rcLand_clean_dome_mat" );
	if (domeMat)
	{
		Texture* envMap = new Texture();
		envMap->SetID( cubemapTarget->GetCubemapID() );
		domeMat->SetEnvMap( envMap );
	}
}

void HQTechnique::Render()
{
	// render shadow cubemaps
	shad->inputs.viewMatrix = Renderer::viewMatrix;
	const uint dynLights = MIN( Renderer::scene->GetLightCount(), MAXDYNAMIC );
	for( uint lightIdx = 0; lightIdx < dynLights; lightIdx++ )
	{
		shad->SetTarget( shadowCubeTarget[lightIdx] );
		shad->inputs.position = Renderer::statics.light[lightIdx].pos;
		shad->Execute();
		Renderer::statics.shadowMapID[lightIdx] = shadowCubeTarget[lightIdx]->GetCubemapID();
	}
	// do a z-prepass (replaces sao stage 1)
	saoLinearZTarget->Clear();
	prez->SetTarget( saoLinearZTarget );
	prez->Execute();
	// sao stage 2: build mipmap for linear z values (minify)
	sao2->inputs.data = saoLinearZTarget->GetColormapID();
	sao2->SetTarget( saoLinearZTarget );
	sao2->Execute();
	// sao stage 3: calculate noisy AO
	sao3->inputs.depth = saoLinearZTarget->GetColormapID();
	sao3->SetTarget( saoNoisyAOTarget );
	sao3->Execute();
#ifdef UNFILTEREDAO
	finao->inputs.color = saoNoisyAOTarget->GetColormapID();
	finao->SetTarget( screenBuffer );
	finao->Execute();
	return;
#endif
	// sao stage 4: horizontal blur
	sao4->inputs.data = saoNoisyAOTarget->GetColormapID();
	sao4->inputs.axis = 0;
	sao4->SetTarget( saoHSmoothTarget );
	sao4->Execute();
	// sao stage 4: vertical blur
	sao4->inputs.data = saoHSmoothTarget->GetColormapID();
	sao4->inputs.axis = 1;
	sao4->SetTarget( saoNoisyAOTarget );
	sao4->Execute();
#ifdef JUSTAO
	finao->inputs.color = saoNoisyAOTarget->GetColormapID();
	finao->SetTarget( screenBuffer );
	finao->Execute();
	return;
#endif
	Renderer::AOdata = saoNoisyAOTarget->GetColormapID();
	// prepare cubemaps
	if (firstFrame) 
	{ 
		firstFrame = false;
		cubem->SetTarget( cubemapTarget );
		cubem->inputs.position = vec3( -2, 2, 4 );
		cubem->Execute();
		Material* domeMat = Renderer::scene->FindMaterial( "vehicle_rcLand_clean_dome_mat" );
		if (domeMat)
		{
			domeMat->SetEnvMap( new Texture() );
			domeMat->GetEnvMap()->SetID( cubemapTarget->GetCubemapID() );
			domeMat->SetColor( vec3( 0.7f, 0.5f, 0.1f ) * 0.6f );
		}
	}
	// forward render to postprocess fbo
	fw->inputs.viewMatrix = Renderer::viewMatrix;
	fw->SetTarget( postprocTargetA );
	fw->Execute();
	// screen space reflection
	ssrr->inputs.color = postprocTargetA->GetColormapID();
	ssrr->inputs.depth = saoLinearZTarget->GetColormapID();
	ssrr->SetTarget( reflectionTarget );
	ssrr->Execute();
	// post processing
	fxaa->inputs.color = reflectionTarget->GetColormapID();
	fxaa->inputs.depth = postprocTargetA->GetDepthTextureID();
#ifdef SKIPDOF
	fxaa->SetTarget( screenBuffer );
	fxaa->Execute();
	return;
#else
	fxaa->SetTarget( postprocTargetB );
	fxaa->Execute();
#endif
#ifndef HQDOFEXPERIMENT
	// depth of field
	bokeh->inputs.color = postprocTargetB->GetColormapID();
	bokeh->inputs.depth = saoLinearZTarget->GetColormapID();
	bokeh->SetTarget( screenBuffer );
	bokeh->Execute();
#else
	// hq dof stage 1: calculate smaller color buffer
	resize->inputs.color = postprocTargetB->GetColormapID();
	resize->SetTarget( resizeTarget );
	resize->Execute();
	// hq dof stage 2: execute hqdof pass
	hqdof->inputs.resolution = resizeTarget->GetSize();
	hqdof->inputs.color = resizeTarget->GetColormapID();
	hqdof->inputs.depth = saoLinearZTarget->GetColormapID();
	hqdof->SetTarget( HQDOFTarget );
	hqdof->Execute();
	// combine
	combine->inputs.color = postprocTargetB->GetColormapID();
	combine->inputs.depth = saoLinearZTarget->GetColormapID();
	combine->inputs.dof = HQDOFTarget->GetColormapID();
	combine->SetTarget( screenBuffer );
	combine->Execute();
#endif
}

void BasicTechnique::Init()
{
	// targets
	postprocTargetA = new GenericTarget( 64, 64 );
	postprocTargetB = new GenericTarget( 64, 64 );
	shadowCubeTarget[0] = new ShadowCubeTarget( 1024, 1024 );
	shadowCubeTarget[1] = new ShadowCubeTarget( 1024, 1024 );
	cubemapTarget = new CubemapTarget( 1024, 1024 );
	// render passes
	fxaa = new FXAAPass();
	cubem = new CubemapPass();
	shad = new ShadowCubePass();
	fw = new ForwardPass();
}

void BasicTechnique::Resize( uint _Width, uint _Height )
{
	screenBuffer->Resize( _Width, _Height );
	postprocTargetA->Resize( _Width, _Height );
	postprocTargetB->Resize( _Width, _Height );
}

void BasicTechnique::RenderToCubeMap( const vec3 _Pos )
{
	cubem->SetTarget( cubemapTarget );
	cubem->inputs.position = _Pos;
	cubem->Execute();
	// assign cubemap to specific scene material
	Material* domeMat = Renderer::scene->FindMaterial( "vehicle_rcLand_clean_dome_mat" );
	if (domeMat)
	{
		Texture* envMap = new Texture();
		envMap->SetID( cubemapTarget->GetCubemapID() );
		domeMat->SetEnvMap( envMap );
	}
}

void BasicTechnique::Render()
{
	// render shadow cubemaps
	shad->inputs.viewMatrix = Renderer::viewMatrix;
	const uint dynLights = MIN( Renderer::scene->GetLightCount(), MAXDYNAMIC );
	for( uint lightIdx = 0; lightIdx < dynLights; lightIdx++ )
	{
		shad->SetTarget( shadowCubeTarget[lightIdx] );
		shad->inputs.position = Renderer::statics.light[lightIdx].pos;
		shad->Execute();
		Renderer::statics.shadowMapID[lightIdx] = shadowCubeTarget[lightIdx]->GetCubemapID();
	}
	// prepare cubemaps, clear AO
	if (firstFrame) 
	{ 
		firstFrame = false;
		cubem->SetTarget( cubemapTarget );
		cubem->inputs.position = vec3( -2, 2, 4 );
		cubem->Execute();
		Material* domeMat = Renderer::scene->FindMaterial( "vehicle_rcLand_clean_dome_mat" );
		domeMat->SetEnvMap( new Texture() );
		domeMat->GetEnvMap()->SetID( cubemapTarget->GetCubemapID() );
		domeMat->SetColor( vec3( 0.7f, 0.5f, 0.1f ) * 0.6f );
		glClearColor( 1, 1, 1, 1 );
		postprocTargetB->Bind();
		glClearColor( 0, 0, 0, 0 );
		Renderer::AOdata = postprocTargetB->GetColormapID();
	}
	// forward render to postprocess fbo
	fw->inputs.viewMatrix = Renderer::viewMatrix;
	fw->SetTarget( postprocTargetA );
	fw->Execute();
	// post processing
	fxaa->inputs.color = postprocTargetA->GetColormapID();
	fxaa->inputs.depth = postprocTargetA->GetDepthTextureID();
	fxaa->SetTarget( screenBuffer );
	fxaa->Execute();
}

void StochasticTechnique::Init()
{
	// buffer for light data
	textureBuffer = new TextureBuffer( 1024 );
	Renderer::statics.lightBuffer = textureBuffer;
	// targets
	gbuffer = new GeometryBuffer( 64, 64 );
	postprocTargetA = new GenericTarget( 64, 64 );
	postprocTargetB = new GenericTarget( 64, 64 );
	lightTarget = new GenericTarget( 64, 64 );
	// render passes
	geom = new GeometryPass();
	stoc = new StochasticPass();
	copy = new CopyPass();
	fxaa = new FXAAPass();
	blur = new BlurPass();
	comb = new CombinePass();
}

void StochasticTechnique::Resize( uint _Width, uint _Height )
{
	screenBuffer->Resize( _Width, _Height );
	gbuffer->Resize( _Width, _Height );
	postprocTargetA->Resize( _Width, _Height );
	postprocTargetB->Resize( _Width, _Height );
	lightTarget->Resize( _Width, _Height );
}

bool lightsInitialized = false;

void StochasticTechnique::Render()
{
	// update light buffer
	if (!lightsInitialized)
	{
		Node* light = Renderer::scene->FindNode( "lightset" );
		vector<Triangle> tri;
		vector<Material*> mat;
		light->GatherTriangles( tri, mat );
		vec4* table = (vec4*)Renderer::statics.lightBuffer->GetData();
		for( size_t s = tri.size(), i = 0; i < s; i++ ) 
		{
			table[i * 3 + 0] = vec4( tri[i].v[0], mat[i]->GetColor().r );
			table[i * 3 + 1] = vec4( tri[i].v[1], mat[i]->GetColor().g );
			table[i * 3 + 2] = vec4( tri[i].v[2], mat[i]->GetColor().b );
		}
		Renderer::statics.lightBuffer->Update();
		lightsInitialized = true;
	}
	// render to geometry buffer
	geom->inputs.viewMatrix = Renderer::viewMatrix;
	geom->SetTarget( gbuffer );
	geom->Execute();
	// render stochastic lighting
	stoc->inputs.wpos = gbuffer->GetBufferID( 0 );
	stoc->inputs.normal = gbuffer->GetBufferID( 2 );
	stoc->SetTarget( postprocTargetA );
	stoc->Execute();
	// horizontal blur
	blur->inputs.color = postprocTargetA->GetColormapID();
	blur->inputs.key = gbuffer->GetBufferID( 2 );
	blur->inputs.axis = 0;
	blur->SetTarget( postprocTargetB );
	blur->Execute();
	// sao stage 4: vertical blur
	blur->inputs.color = postprocTargetB->GetColormapID();
	blur->inputs.key = gbuffer->GetBufferID( 2 );
	blur->inputs.axis = 1;
	blur->SetTarget( lightTarget );
	blur->Execute();
	// finalize
	comb->inputs.diffuse = gbuffer->GetBufferID( 1 );
	comb->inputs.light = lightTarget->GetColormapID();
	comb->SetTarget( screenBuffer );
	comb->Execute();
}

// EOF