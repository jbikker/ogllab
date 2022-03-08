#include "precomp.h"

static Quad* quad = 0;
Scene* Pass::scene = 0;

void Pass::PrepareExecute()
{
	// activate shader
	glUseProgram( shader->GetID() );
	// prepare output
	glBindFramebuffer( GL_FRAMEBUFFER, target->GetID() );
    glViewport( 0, 0, target->GetSize().x, target->GetSize().y );
	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
}

void Pass::AttachTexture( uint _Idx, const char* _Name, GLuint _ID, bool _Depth )
{
	glActiveTexture( GL_TEXTURE0 + _Idx );
	glBindTexture( GL_TEXTURE_2D, _ID );
	glUniform1i( glGetUniformLocation( shader->GetID(), _Name ), _Idx );
	if (_Depth)	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
}

DebugPass::DebugPass()
{
	shader = new Shader( "postproc.vert", "debug.frag" );
	if (!quad) quad = new Quad();
}

void DebugPass::Execute()
{
	// default preparations
	PrepareExecute();
	// attach inputs
	AttachTexture( 0, "data", inputs.data );
	// set uniforms
	mat4 matrix( 1 );
	glUniformMatrix4fv( glGetUniformLocation( shader->GetID(), "view" ), 1, GL_FALSE, &matrix[0][0] );
	// render
	quad->Render();
}

BlurPass::BlurPass()
{
	shader = new Shader( "gaussian.vert", "gaussian.frag" );
	if (!quad) quad = new Quad();
}

void BlurPass::Execute()
{
	// default preparations
	PrepareExecute();
	// attach inputs
	AttachTexture( 0, "source", inputs.color );
	AttachTexture( 1, "depth", inputs.key, true );
	// set uniforms
	mat4 matrix( 1 );
	ivec2 screenSize = target->GetSize();
	glUniformMatrix4fv( glGetUniformLocation( shader->GetID(), "view" ), 1, GL_FALSE, &matrix[0][0] );
	ivec2 axis = inputs.axis == 0 ? vec2( 1, 0 ) : vec2( 0, 1 );
	glUniform2i( glGetUniformLocation( shader->GetID(), "axis" ), axis.x, axis.y );
	// render
	quad->Render();
}

FinalizeAOPass::FinalizeAOPass()
{
	shader = new Shader( "finalizeao.vert", "finalizeao.frag" );
	if (!quad) quad = new Quad();
}

void FinalizeAOPass::Execute()
{
	// default preparations
	PrepareExecute();
	// attach inputs
	AttachTexture( 0, "color", inputs.color );
	// set uniforms
	mat4 matrix( 1 );
	ivec2 screenSize = target->GetSize();
	glUniformMatrix4fv( glGetUniformLocation( shader->GetID(), "view" ), 1, GL_FALSE, &matrix[0][0] );
	// render
	quad->Render();
}

CopyPass::CopyPass()
{
	shader = new Shader( "copy.vert", "copy.frag" );
	if (!quad) quad = new Quad();
}

void CopyPass::Execute()
{
	// default preparations
	PrepareExecute();
	// attach inputs
	AttachTexture( 0, "color", inputs.color );
	// set uniforms
	mat4 matrix( 1 );
	ivec2 screenSize = target->GetSize();
	glUniformMatrix4fv( glGetUniformLocation( shader->GetID(), "view" ), 1, GL_FALSE, &matrix[0][0] );
	// render
	quad->Render();
}

PrePass::PrePass()
{
	shader = new Shader( "linearz.vert", "linearz.frag" );
	glUniformBlockBinding( shader->GetID(), glGetUniformBlockIndex( shader->GetID(), "TransformBlock" ), 1 );
}

void PrePass::Execute()
{
	// activate shader
	glUseProgram( shader->GetID() );
	Renderer::statics.shadowShaderID = shader->GetID();
	// prepare output
	glBindFramebuffer( GL_FRAMEBUFFER, target->GetID() );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glClear( GL_DEPTH_BUFFER_BIT );
	// set shader parameters
	float znear = -0.1f, zfar = -500.0f;
	glUniform3f( glGetUniformLocation( shader->GetID(), "clipInfo" ), znear * zfar, znear - zfar, zfar );
	// render to shadow map
	ivec2 screenSize = target->GetSize();
	glViewport( 0, 0, screenSize.x, screenSize.y );
	Frustum viewFrustum( Renderer::viewMatrix );
	Renderer::frustum = &viewFrustum;
	scene->RenderShadow( Renderer::viewMatrix );
}

FXAAPass::FXAAPass()
{
	shader = new Shader( "postproc.vert", "postproc.frag" );
	if (!quad) quad = new Quad();
}

void FXAAPass::Execute()
{
	// default preparations
	PrepareExecute();
	// attach inputs
	AttachTexture( 0, "color", inputs.color );
	AttachTexture( 1, "depth", inputs.depth );
	// set uniforms
	mat4 matrix( 1 );
	glUniform2f( glGetUniformLocation( shader->GetID(), "screenSize" ), (float)target->GetSize().x, (float)target->GetSize().y );
	glUniformMatrix4fv( glGetUniformLocation( shader->GetID(), "view" ), 1, GL_FALSE, &matrix[0][0] );
	// render
	quad->Render();
}

BokehPass::BokehPass()
{
	shader = new Shader( "dof.vert", "dof.frag" );
	if (!quad) quad = new Quad();
	// defaults
	inputs.fstops = 1.0f;
	inputs.focalDepth = 100;
	inputs.focalLength = 50;
}

void BokehPass::Execute()
{
	// default preparations
	PrepareExecute();
	// attach inputs
	AttachTexture( 0, "color", inputs.color );
	AttachTexture( 1, "depth", inputs.depth );
	// set uniforms
	mat4 matrix( 1 );
	glUniform2f( glGetUniformLocation( shader->GetID(), "screenSize" ), (float)target->GetSize().x, (float)target->GetSize().y );
	glUniformMatrix4fv( glGetUniformLocation( shader->GetID(), "view" ), 1, GL_FALSE, &matrix[0][0] );
	glUniform1f( glGetUniformLocation( shader->GetID(), "focalDepth" ), inputs.focalDepth );
	glUniform1f( glGetUniformLocation( shader->GetID(), "focalLength" ), inputs.focalLength );
	glUniform1f( glGetUniformLocation( shader->GetID(), "fstop" ), inputs.fstops );
	// render
	quad->Render();
}

HQDOFPass::HQDOFPass()
{
	inputs.resolution = vec2( 0, 0 );
	DOFGrid = new Mesh();
	shader = new Shader( "hqdof.vert", "hqdof.frag" );
	bokeh = new Texture();
	bokeh->Load( "data/bokeh.tga" );
}

void HQDOFPass::Execute()
{
	// default preparations
	PrepareExecute();
	// update mesh
	if (inputs.resolution != resolution)
	{
		resolution = inputs.resolution;
		DOFGrid->CreateDOFGrid( resolution );
		glUniform2i( glGetUniformLocation( shader->GetID(), "screenSize" ), resolution.x, resolution.y );
		glUniform1f( glGetUniformLocation( shader->GetID(), "focalDepth" ), -80 );
	}
	// attach inputs
	AttachTexture( 0, "bokeh", bokeh->GetID() );
	AttachTexture( 1, "color", inputs.color );
	AttachTexture( 2, "depth", inputs.depth );
	// render
	glClear( GL_COLOR_BUFFER_BIT );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	glBindVertexArray( DOFGrid->GetID() );
	glDrawElements( GL_TRIANGLES, DOFGrid->GetTriCount() * 3, GL_UNSIGNED_INT, 0 );
	glUseProgram( 0 );
	glDisable( GL_BLEND );
}

HQDOFCombine::HQDOFCombine()
{
	shader = new Shader( "hqdofcombine.vert", "hqdofcombine.frag" );
	if (!quad) quad = new Quad();
}

void HQDOFCombine::Execute()
{
	// default preparations
	PrepareExecute();
	// attach inputs
	AttachTexture( 0, "color", inputs.color );
	AttachTexture( 1, "depth", inputs.depth );
	AttachTexture( 2, "dof", inputs.dof );
	// set uniforms
	mat4 matrix( 1 );
	glUniformMatrix4fv( glGetUniformLocation( shader->GetID(), "view" ), 1, GL_FALSE, &matrix[0][0] );
	glUniform2i( glGetUniformLocation( shader->GetID(), "screenSize" ), target->GetSize().x, target->GetSize().y );
	glUniform1f( glGetUniformLocation( shader->GetID(), "focalDepth" ), -80 );
	// render
	quad->Render();
}

void CubemapPass::Execute()
{
	const mat4 projection = perspective( PI / 2, 1.0f, 0.01f, 500.0f );
	for( int i = 0; i < 6; i++ )
	{
		((CubemapTarget*)target)->Bind( i );
		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );
		mat4 side;
		if (i == 0) side = lookAt( inputs.position, inputs.position + vec3( 1, 0, 0 ), vec3( 0, -1, 0 ) );
		else if (i == 1) side = lookAt( inputs.position, inputs.position - vec3( 1, 0, 0 ), vec3( 0, -1, 0 ) );
		else if (i == 2) side = lookAt( inputs.position, inputs.position + vec3( 0, 1, 0 ), vec3( 0, 0, -1 ) );
		else if (i == 3) side = lookAt( inputs.position, inputs.position - vec3( 0, 1, 0 ), vec3( 0, 0, -1 ) );
		else if (i == 4) side = lookAt( inputs.position, inputs.position + vec3( 0, 0, 1 ), vec3( 0, -1, 0 ) );
		else if (i == 5) side = lookAt( inputs.position, inputs.position - vec3( 0, 0, 1 ), vec3( 0, -1, 0 ) );
		mat4 viewMatrix = projection * side;
		Frustum viewFrustum( viewMatrix );
		Renderer::frustum = &viewFrustum;
		scene->Render( RENDER_DIRECT, mat4( 1 ), viewMatrix, mat4( 1 ) );
	}
}

ShadowCubePass::ShadowCubePass()
{
	inputs.nearFar = vec2( 0.1f, 200.0f );
	shader = new Shader( "shadowcube.vert", "shadowcube.frag" );
	glUniformBlockBinding( shader->GetID(), glGetUniformBlockIndex( shader->GetID(), "TransformBlock" ), 1 );
}

void ShadowCubePass::Execute()
{
	glUseProgram( shader->GetID() );
	Renderer::statics.shadowShaderID = shader->GetID();
	glUniform2f( glGetUniformLocation( shader->GetID(), "nearfar" ), inputs.nearFar.x, inputs.nearFar.y );
	glUniform4f( glGetUniformLocation( shader->GetID(), "lightPos" ), inputs.position.x, inputs.position.y, inputs.position.z, 1 );
	const mat4 projection = perspective( PI / 2, 1.0f, 0.1f, 200.0f );
	mat4 view[6] = {
		projection * lookAt( inputs.position, inputs.position + vec3( 1, 0, 0 ), vec3( 0, -1, 0 ) ),
		projection * lookAt( inputs.position, inputs.position - vec3( 1, 0, 0 ), vec3( 0, -1, 0 ) ),
		projection * lookAt( inputs.position, inputs.position + vec3( 0, 1, 0 ), vec3( 0, 0, -1 ) ),
		projection * lookAt( inputs.position, inputs.position - vec3( 0, 1, 0 ), vec3( 0, 0, -1 ) ),
		projection * lookAt( inputs.position, inputs.position + vec3( 0, 0, 1 ), vec3( 0, -1, 0 ) ),
		projection * lookAt( inputs.position, inputs.position - vec3( 0, 0, 1 ), vec3( 0, -1, 0 ) )
	};
	Frustum viewFrustum( inputs.viewMatrix );
	for( int i = 0; i < 6; i++ )
	{
		Frustum cubeFrustum( view[i] );
		if (viewFrustum.Intersect( cubeFrustum ))
		{
			((ShadowCubeTarget*)target)->Bind( i, inputs.position );
			Renderer::statics.view = view[i];
			Renderer::frustum = &cubeFrustum;
			scene->RenderShadowCube( mat4( 1 ) );
		}
	}
}

ShadowMapPass::ShadowMapPass()
{
	inputs.direction = vec3( 0.5f, 2, 2 );
	shader = new Shader( "shadow.vert", "shadow.frag" );
	glUniformBlockBinding( shader->GetID(), glGetUniformBlockIndex( shader->GetID(), "TransformBlock" ), 1 );
}

void ShadowMapPass::Execute()
{
	// activate shader
	glUseProgram( shader->GetID() );
	Renderer::statics.shadowShaderID = shader->GetID();
	// prepare output
	ivec2 screenSize = target->GetSize();
	glBindFramebuffer( GL_FRAMEBUFFER, target->GetID() );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	// prepare shadow mapping
	mat4 lightOrtho = ortho<float>( -inputs.size, inputs.size, -inputs.size, inputs.size, -1.0f, 10.0f );
	vec3 invL = normalize( inputs.direction );
	mat4 bias( 0.5, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 1 );
	mat4 lightMatrix = lightOrtho * lookAt( invL, vec3( 0, 0, 0 ), vec3( 0, 1, 0 ) );
	// render to shadow map
	scene->RenderShadow( lightMatrix );
}

SAOMinifyPass::SAOMinifyPass()
{
	shader = new Shader( "sao/sao.vrt", "sao/sao_minify.pix" );
	if (!quad) quad = new Quad();
}

void SAOMinifyPass::Execute()
{
	// activate shader
	GLuint shaderID = shader->GetID();
	glUseProgram( shaderID );
	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	// attach inputs
	AttachTexture( 0, "data", inputs.data );
	// set uniforms
	mat4 matrix( 1 );
	ivec2 screenSize = target->GetSize();
	glUniformMatrix4fv( glGetUniformLocation( shaderID, "view" ), 1, GL_FALSE, &matrix[0][0] );
	// loop over MIP levels
	glBindFramebuffer( GL_FRAMEBUFFER, target->GetID() );
	for ( int i = 1; i <= 5; i++ )
	{
		// set uniforms
		glUniform1i( glGetUniformLocation( shaderID, "previousMIPNumber" ), i - 1 );
		// prepare output
		glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, inputs.data, i );
		screenSize.x >>= 1, screenSize.y >>= 1;
		glUniform2f( glGetUniformLocation( shaderID, "screenSize" ), (float)screenSize.x, (float)screenSize.y );
		glViewport( 0, 0, screenSize.x, screenSize.y );
		// render
		quad->Render();
	}
	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, inputs.data, 0 );
}

SAONoisyAOPass::SAONoisyAOPass()
{
	shader = new Shader( "postproc.vert", "sao/sao_ao.pix" );
	if (!quad) quad = new Quad();
	// defaults
	inputs.scale = 500.0f;
	inputs.radius = 50.0f;
	inputs.bias = 0.01f;
	inputs.intensity = 4.8f;
}

void SAONoisyAOPass::Execute()
{
	// default preparations
	PrepareExecute();
	// attach inputs
	AttachTexture( 0, "CS_Z_buffer", inputs.depth, true );
	// set uniforms
	mat4 matrix( 1 );
    ivec2 screenSize = target->GetSize();
	float projScale = 500.0f * (screenSize.y / 1080.0f);
	glUniformMatrix4fv( glGetUniformLocation( shader->GetID(), "view" ), 1, GL_FALSE, &matrix[0][0] );
	glUniform1f( glGetUniformLocation( shader->GetID(), "projScale" ), projScale );
	glUniform1f( glGetUniformLocation( shader->GetID(), "radius" ), inputs.radius );
	glUniform1f( glGetUniformLocation( shader->GetID(), "bias" ), inputs.bias );
	float r = inputs.radius;
	glUniform1f( glGetUniformLocation( shader->GetID(), "intensityDivR6" ), inputs.intensity / (r * r * r * r * r * r) );
	mat4 P = Renderer::projMatrix;
	vec4 v( -2.0f / (screenSize.x * P[0][0]), -2.0f / (screenSize.y * P[1][1]), (1.0f - P[0][2]) / P[0][0], (1.0f + P[1][2]) / P[1][1] );
	glUniform4f( glGetUniformLocation( shader->GetID(), "projInfo" ), v.x, v.y, v.z, v.w );
	// render
	quad->Render();
}

SAOBlurPass::SAOBlurPass()
{
	shader = new Shader( "sao/sao.vrt", "sao/sao_blur.pix" );
	if (!quad) quad = new Quad();
}

void SAOBlurPass::Execute()
{
	// default preparations
	PrepareExecute();
	// attach inputs
	AttachTexture( 0, "source", inputs.data );
	// set uniforms
	mat4 matrix( 1 );
	ivec2 screenSize = target->GetSize();
	glUniformMatrix4fv( glGetUniformLocation( shader->GetID(), "view" ), 1, GL_FALSE, &matrix[0][0] );
	ivec2 axis = inputs.axis == 0 ? vec2( 1, 0 ) : vec2( 0, 1 );
	glUniform2i( glGetUniformLocation( shader->GetID(), "axis" ), axis.x, axis.y );
	// render
	quad->Render();
}

SSRRPass::SSRRPass()
{
	shader = new Shader( "ssrr.vert", "ssrr.frag" );
	if (!quad) quad = new Quad();
}

void SSRRPass::Execute()
{
	// default preparations
	PrepareExecute();
	// attach inputs
	AttachTexture( 0, "color", inputs.color );
	AttachTexture( 1, "depth", inputs.depth );
	// set uniforms
	mat4 matrix( 1 );
	ivec2 screenSize = target->GetSize();
	glUniformMatrix4fv( glGetUniformLocation( shader->GetID(), "view" ), 1, GL_FALSE, &matrix[0][0] );
	mat4 P = Renderer::projMatrix;
	vec4 v( -2.0f / (screenSize.x * P[0][0]), -2.0f / (screenSize.y * P[1][1]), (1.0f - P[0][2]) / P[0][0], (1.0f + P[1][2]) / P[1][1] );
	glUniform4f( glGetUniformLocation( shader->GetID(), "projInfo" ), v.x, v.y, v.z, v.w );
	// render
	quad->Render();
}

void ForwardPass::Execute()
{
	// prepare output
	ivec2 screenSize = target->GetSize();
	glBindFramebuffer( GL_FRAMEBUFFER, target->GetID() );
    glViewport( 0, 0, target->GetSize().x, target->GetSize().y );
	glDisable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	// render
	Frustum viewFrustum( inputs.viewMatrix );
	Renderer::frustum = &viewFrustum;
	scene->Render( RENDER_SHADOWCUBE, mat4( 1 ), inputs.viewMatrix, mat4( 1 ) );
}

void GeometryPass::Execute()
{
	// prepare output
	ivec2 screenSize = target->GetSize();
	glBindFramebuffer( GL_FRAMEBUFFER, target->GetID() );
    glViewport( 0, 0, target->GetSize().x, target->GetSize().y );
	glDisable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	// render
	Frustum viewFrustum( inputs.viewMatrix );
	Renderer::frustum = &viewFrustum;
	scene->Render( RENDER_GEOMETRY, mat4( 1 ), inputs.viewMatrix, mat4( 1 ) );
}

StochasticPass::StochasticPass()
{
	shader = new Shader( "stochastic.vert", "stochastic.frag" );
	if (!quad) quad = new Quad();
}

void StochasticPass::Execute()
{
	// default preparations
	PrepareExecute();
	// attach inputs
	AttachTexture( 0, "wpos", inputs.wpos );
	AttachTexture( 1, "normal", inputs.normal );
	glUniform1i( glGetUniformLocation( shader->GetID(), "data" ), 2 );
	glActiveTexture( GL_TEXTURE2 );
	glBindTexture( GL_TEXTURE_BUFFER, Renderer::statics.lightBuffer->GetTextureID() );
	// set uniforms
	mat4 matrix( 1 );
	ivec2 screenSize = target->GetSize();
	glUniformMatrix4fv( glGetUniformLocation( shader->GetID(), "view" ), 1, GL_FALSE, &matrix[0][0] );
	// render
	quad->Render();
}

CombinePass::CombinePass()
{
	shader = new Shader( "combine.vert", "combine.frag" );
	if (!quad) quad = new Quad();
}

void CombinePass::Execute()
{
	// default preparations
	PrepareExecute();
	// attach inputs
	AttachTexture( 0, "diffuse", inputs.diffuse );
	AttachTexture( 1, "light", inputs.light );
	// set uniforms
	mat4 matrix( 1 );
	ivec2 screenSize = target->GetSize();
	glUniformMatrix4fv( glGetUniformLocation( shader->GetID(), "view" ), 1, GL_FALSE, &matrix[0][0] );
	// render
	quad->Render();
}

// EOF