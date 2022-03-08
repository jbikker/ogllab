#include "precomp.h"

Scene* Node::scene = 0; // for light array access

Node::Node() : name( 0 )
{
}

void Node::SetFlags( NodeFlags _Flags )
{
	flags = _Flags;
	for( size_t s = child.size(), i = 0; i < s; i++ ) child[i]->SetFlags( flags );
}

void Node::SetName( const char* _Name )
{
	delete name;
	name = new char[strlen( _Name ) + 1];
	strcpy( name, _Name );
}

Node* Node::Find( const char* _Name )
{
	if (name) if (!strcmp( name, _Name )) return this;
	Node* found = 0;
	for(size_t s = child.size(), i = 0; i < s; i++ ) if (found = child[i]->Find( _Name )) break;
	return found;
}

void Node::AddChild( Node* _Child )
{
	child.push_back( _Child );
}

TransformNode::TransformNode() : transform( mat4( 1 ) )
{
}

void TransformNode::Render( const RenderType _Type, mat4& _World, mat4& _View, mat4& _Light )
{
	Renderer::statics.view = _View * transform;
	Renderer::statics.world = _World * transform;
	Renderer::statics.transformUBO->Update();
	mat4 lightMatrix = _Light * transform;
	for (size_t s = child.size(), i = 0; i < s; i++ ) child[i]->Render( _Type, Renderer::statics.world, Renderer::statics.view, lightMatrix );
}

void TransformNode::Gather( const RenderType _Type, uint _Transform, mat4& _World, mat4& _View, mat4& _Light )
{
	DrawTransform dt;
	dt.world = _World * transform;
	dt.view = _View * transform;
	dt.light = _Light * transform;
	uint currentTransform = (int)Renderer::drawTransform.size();
	Renderer::drawTransform.push_back( dt );
	for (size_t s = child.size(), i = 0; i < s; i++ ) child[i]->Gather( _Type, currentTransform, dt.world, dt.view, dt.light );
}

void TransformNode::RenderShadow( mat4& _View )
{
	Renderer::statics.view = _View * transform;
	Renderer::statics.transformUBO->Update();
	for ( size_t s = child.size(), i = 0; i < s; i++ ) child[i]->RenderShadow( Renderer::statics.view );
}

void TransformNode::RenderShadowCube( mat4& _World )
{
	mat4 worldTransform = _World * transform;
	Renderer::statics.world = worldTransform * Renderer::statics.view;
	Renderer::statics.transformUBO->Update();
	for (size_t s = child.size(), i = 0; i < s; i++ ) child[i]->RenderShadowCube( worldTransform );
}

void TransformNode::GatherTriangles( vector<Triangle>& _Tri, vector<Material*>& _Mat )
{
	for (size_t s = child.size(), i = 0; i < s; i++ ) child[i]->GatherTriangles( _Tri, _Mat );
}

MeshBase::MeshBase() : material( 0 ), tris( 0 ), key( 0 )
{
	vertexBuffer = UVBuffer = normalBuffer = indexBuffer = 0;
}

MeshBase::~MeshBase()
{
	delete material;
}

GLuint MeshBase::CreateVBO( const GLfloat* _Data, const uint _Size )
{
	GLuint bufferID;
	glGenBuffers( 1, &bufferID );
	glBindBuffer( GL_ARRAY_BUFFER, bufferID );
	glBufferData( GL_ARRAY_BUFFER, _Size, _Data, GL_STATIC_DRAW );
	return bufferID;
}

void MeshBase::BindVBO( const uint _Idx, const uint _Elements, const GLuint _ID )
{
	glEnableVertexAttribArray( _Idx );
	glBindBuffer( GL_ARRAY_BUFFER, _ID );
	glVertexAttribPointer( _Idx, _Elements, GL_FLOAT, GL_FALSE, 0, (void*)0 );
}

void MeshBase::RenderShadow( mat4& _View )
{
	glBindVertexArray( vao );
	glDrawElements( GL_TRIANGLES, tris * 3, GL_UNSIGNED_INT, 0 );
}

void MeshBase::RenderShadowCube( mat4& _World )
{
	if ((flags == NODE_DYNAMIC) || (Renderer::frustum->Intersect( bounds )))
	{
		glBindVertexArray( vao );
		glDrawElements( GL_TRIANGLES, tris * 3, GL_UNSIGNED_INT, 0 );
	}
}

void MeshBase::Render( const RenderType _Type, mat4& _World, mat4& _View, mat4& _Light )
{
	if ((flags == NODE_DYNAMIC) || (Renderer::frustum->Intersect( bounds )))
	{
		glBindVertexArray( vao );
		material->Bind( _Type );
		glDrawElements( GL_TRIANGLES, tris * 3, GL_UNSIGNED_INT, 0 );
	}
}

void MeshBase::CalculateKey( const RenderType _Type )
{
	uint shader = material->GetShader( _Type )->GetID();
	uint texture = material->GetTexture() ? material->GetTexture()->GetID() : 0;
	uint normals = material->GetNormalMap() ? material->GetNormalMap()->GetID() : 0;
	uint envmap = material->GetEnvMap() ? material->GetEnvMap()->GetID() : 0;
	key = texture + (normals << 10) + (shader << 20) + (envmap << 28);
}

void MeshBase::Gather( const RenderType _Type, uint _Transform, mat4& _World, mat4& _View, mat4& _Light )
{
	if ((flags == NODE_DYNAMIC) || (Renderer::frustum->Intersect( bounds )))
	{
		if (!key) CalculateKey( _Type );
		DrawCall dc;
		dc.transform = _Transform;
		dc.mesh = this;
		dc.key = key;
		Renderer::drawCall.push_back( dc );
	}
}

void MeshBase::GatherTriangles( vector<Triangle>& _Tri, vector<Material*>& _Mat )
{
	for( uint i = 0; i < tris; i++ )
	{
		Triangle t;
		t.v[0] = vertices[indices[i * 3 + 0]];
		t.v[1] = vertices[indices[i * 3 + 1]];
		t.v[2] = vertices[indices[i * 3 + 2]];
		_Tri.push_back( t );
		_Mat.push_back( material );
	}
}

void Mesh::CreateDOFGrid( const ivec2 _Res )
{
	const uint size = _Res.x * _Res.y * 3;
	uint* idx = new uint[size];
	for( uint i = 0; i < size; i++ ) idx[i] = i;
	if (!indexBuffer) 
	{
		// first time: generate buffer and vertex array object
		glGenBuffers( 1, &indexBuffer );
		glGenVertexArrays( 1, &vao );
	}
	// fill buffer with data
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, size * 4, idx, GL_STATIC_DRAW );
	// create vao
	glBindVertexArray( vao );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );
	glBindVertexArray( 0 );
	delete idx;
	tris = _Res.x * _Res.y;
}

void Mesh::CreateFromArrays( vector<vec3>& _Verts, vector<vec3>& _Normals, vector<vec2>& _UVs, vector<uint>& _Indices )
{
	// copy data to data members
	vertices = _Verts;
	normals = _Normals;
	uvs = _UVs;
	indices = _Indices; 
	// create ogl buffers
	tris = (int)_Indices.size() / 3;
	if (_UVs.size() > 0) UVBuffer = CreateVBO( (GLfloat*)&_UVs[0], (uint)_UVs.size() * sizeof( vec2 ) );
	vertexBuffer = CreateVBO( (GLfloat*)&_Verts[0], (uint)_Verts.size() * sizeof( vec3 ) );
	normalBuffer = CreateVBO( (GLfloat*)&_Normals[0], (uint)_Normals.size() * sizeof( vec3 ) );
	glGenBuffers( 1, &indexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, 4 * (uint)_Indices.size(), &_Indices[0], GL_STATIC_DRAW );
	// generate vao
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
	uint idx = 0;
	if (vertexBuffer) BindVBO( idx++, 3, vertexBuffer );
	if (normalBuffer) BindVBO( idx++, 3, normalBuffer );
	if (UVBuffer) BindVBO( idx++, 2, UVBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );
	glBindVertexArray( 0 );
	for( uint i = 0; i < idx; i++ ) glDisableVertexAttribArray( i );
	// calculate bounds
	vec3& b1 = bounds.min, &b2 = bounds.max;
	b1 = vec3(  MAXFLOAT ), b2 = vec3( -MAXFLOAT );
	for( size_t s = _Verts.size(), i = 0; i < s; i++ )
		b1.x = MIN( b1.x, _Verts[i].x ), b2.x = MAX( b2.x, _Verts[i].x ),
		b1.y = MIN( b1.y, _Verts[i].y ), b2.y = MAX( b2.y, _Verts[i].y ),
		b1.z = MIN( b1.z, _Verts[i].z ), b2.z = MAX( b2.z, _Verts[i].z );
	// default to static
	flags = NODE_STATIC;
}

void Mesh::CreateFromAssimp( const aiMesh* _Mesh )
{
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
	vector<vec3> pos, normal;
	vector<vec2> uv;
	vector<uint> idx;
	pos.reserve( _Mesh->mNumVertices );
	normal.reserve( _Mesh->mNumVertices );
	uv.reserve( _Mesh->mNumVertices );
	idx.reserve( _Mesh->mNumFaces * 3 );
	for( uint i = 0; i < _Mesh->mNumVertices; i++) 
	{
		const aiVector3D& P = _Mesh->mVertices[i];
		const aiVector3D& N = _Mesh->mNormals[i];
		pos.push_back( vec3( P.x, P.y, P.z ) );
		normal.push_back( vec3( N.x, N.y, N.z ) );
	}
	if (_Mesh->HasTextureCoords( 0 )) for( uint i = 0; i < _Mesh->mNumVertices; i++) 
	{
		const aiVector3D& UV = _Mesh->mTextureCoords[0][i];
		uv.push_back( vec2( UV.x, UV.y ) );        
	}
	for (uint i = 0; i < _Mesh->mNumFaces ; i++) 
	{
		const aiFace& tri = _Mesh->mFaces[i];
		idx.push_back( tri.mIndices[0] );
		idx.push_back( tri.mIndices[1] );
		idx.push_back( tri.mIndices[2] );
	}
}

Quad::Quad()
{
	// hardcoded arrays
	const GLfloat verts[] = { -1, -1, 0, 1, -1, 0, -1, 1, 0, 1, -1, 0, -1, 1, 0, 1, 1, 0 };
	const GLfloat uvdata[] = { 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1 };
	// generate buffers
	vertexBuffer = CreateVBO( verts, sizeof( verts ) );
	UVBuffer = CreateVBO( uvdata, sizeof( uvdata ) );
	tris = 2;
	// material
	material = 0;
	// generate vao
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
	BindVBO( 0, 3, vertexBuffer );
	BindVBO( 1, 2, UVBuffer );
	glBindVertexArray( 0 );
	glDisableVertexAttribArray( 0 );
	glDisableVertexAttribArray( 1 );
}

void Quad::Render()
{
	glBindVertexArray( vao );
	glDrawArrays( GL_TRIANGLES, 0, tris * 3 );
	glBindVertexArray( 0 );
}

SkyBox::SkyBox()
{
	// hardcoded arrays
	const GLfloat A = 0, B = -50, C = 50, verts[] = {   B, B, B, B, B, C, B, C, C, C, C, B, B, B, B, B, C, B, C, B, C, B, B, B, C, B, 
		B, C, C, B, C, B, B, B, B, B, B, B, B, B, C, C, B, C, B, C, B, C, B, B, C, B, B, B, B, C, C, B, B, C, C, B, C, C, C, C, C, 
		B, B, C, C, B, C, B, B, C, C, C, C, B, C, C, C, C, C, C, B, B, C, B, C, C, C, B, C, B, B, C, C, C, C, C, B, C, C, C, B, C };
	// generate buffers
	vertexBuffer = CreateVBO( verts, sizeof( verts ) );
	tris = 12;
	// material
	material = new Material();
	Texture* cube = new Texture();
	const char* files[6] = { "data/sky1.png", "data/sky4.png", "data/sky3.png", "data/sky6.png", "data/sky5.png", "data/sky2.png" };
	cube->LoadCubemap( files );
	material->SetEnvMap( cube );
	material->SetShader( RENDER_DIRECT, new Shader( "sky.vert", "sky.frag" ) );
	material->SetShader( RENDER_GEOMETRY, new Shader( "sky_G.vert", "sky_G.frag" ) );
	// angle
	angle = 0;
	// generate vao
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
	BindVBO( 0, 3, vertexBuffer );
	glBindVertexArray( 0 );
	glDisableVertexAttribArray( 0 );
}

void SkyBox::Render( const RenderType _Type, mat4& _World, mat4& _View, mat4& _Light )
{
	// disable z-buffering for skybox
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_CULL_FACE );
	glBindVertexArray( vao );
	// bind material
	if (_Type == RENDER_GEOMETRY) material->Bind( RENDER_GEOMETRY ); else material->Bind( RENDER_SKY );
	const GLuint activeShaderID = material->GetActiveShader()->GetID();
	// set transform
	mat4 dir = _View;
	float* fdir = (float*)&dir;
	fdir[12] = fdir[13] = fdir[14] = 0;
	const GLuint matrixID = glGetUniformLocation( activeShaderID, "view" );
	glUniformMatrix4fv( matrixID, 1, GL_FALSE, &dir[0][0] );
	if (_Type == RENDER_GEOMETRY) glUniformMatrix4fv( glGetUniformLocation( activeShaderID, "world" ), 1, GL_FALSE, &_World[0][0] );
	// render
	glDrawArrays( GL_TRIANGLES, 0, tris * 3 );
	// unbind shader
	material->Unbind();
	// restore z-buffering
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
}

void SkyBox::Gather( const RenderType _Type, uint _Transform, mat4& _World, mat4& _View, mat4& _Light )
{
	// should happen first anyway
	Render( _Type, _World, _View, _Light );
}

// EOF