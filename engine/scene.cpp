#include "precomp.h"

#define LIGHTSCALE	3.5f

Scene::Scene() : scenePath( 0 )
{
	// init scene
	nodeList.push_back( new SkyBox() );
	skyCubeMap = ((SkyBox*)nodeList[0])->GetMaterial()->GetEnvMap();
	// nodeList.push_back( Load( "data/scenes/legocar/legocar.obj", 1.0f ) );
	// nodeList.push_back( Load( "data/scenes/crytek-sponza/sponza2.obj", 1.0f ) );
	// nodeList.push_back( Load( "data/scenes/unity/unity.obj", 1.0f ) );
	nodeList.push_back( Load( "data/scenes/unity_full/unityScene.obj", 1.0f ) );
	TransformNode* light;
	nodeList.push_back( light = Load( "data/objects/sphere/sphere.obj", 0.1f ) );
	light->SetTransform( translate( mat4( 1 ), vec3( 3, 6, 12.5f ) ) );
	light->SetFlags( NODE_DYNAMIC );
	lightList.push_back( new PointLight( vec3( 3, 6, 12 ), LIGHTSCALE * 0.5f * vec3( 30, 30, 25 ), light ) );
	nodeList.push_back( light = Load( "data/objects/sphere/sphere.obj", 0.1f ) );
	light->SetTransform( translate( mat4( 1 ), vec3( -2, 3, -10 ) ) );
	light->SetFlags( NODE_DYNAMIC );
	lightList.push_back( new PointLight( vec3( -2, 3, -10 ), LIGHTSCALE * vec3( 20, 20, 15 ), light ) );
#if (TECHNIQUE == 3)
	nodeList.push_back( light = Load( "data/objects/lights/lightset.obj", 0.05f ) );
	light->SetName( "lightset" );
#endif
}

Scene::~Scene()
{
	for (size_t s = nodeList.size(), i = 0; i < s; i++) delete nodeList[i];
}

void Scene::ExtractScenePath( const char* _File )
{
	char tmp[2048], * lastSlash = tmp;
	strcpy( tmp, _File );
	while (strstr( lastSlash, "/" )) lastSlash = strstr( lastSlash, "/" ) + 1;
	while (strstr( lastSlash, "\\" )) lastSlash = strstr( lastSlash, "\\" ) + 1;
	*lastSlash = 0;
	delete scenePath;
	scenePath = new char[strlen( tmp ) + 1];
	strcpy( scenePath, tmp );
}

Node* Scene::FindNode( const char* _Name )
{
	Node* found = 0;
	for (size_t s = nodeList.size(), i = 0; i < s; i++) if (found = nodeList[i]->Find( _Name )) break;
	return found;
}

Material* Scene::FindMaterial( const char* _Name )
{
	for (size_t s = matList.size(), i = 0; i < s; i++) if (!strcmp( matList[i]->GetName(), _Name )) return matList[i];
	return 0;
}

Texture* Scene::FindTexture( const char* _Name )
{
	for (size_t s = texList.size(), i = 0; i < s; i++) if (!strcmp( texList[i]->GetName(), _Name )) return texList[i];
	return 0;
}

TransformNode* Scene::Load( const char* _File, const float _Scale )
{
	ExtractScenePath( _File );
	if (strstr( _File, ".bin" ) == (_File + strlen( _File ) - 4)) return LoadBIN( _File, _Scale );
	else if (strstr( _File, ".obj" ) == (_File + strlen( _File ) - 4)) return LoadOBJ( _File, _Scale );
	else return LoadAssimp( _File, _Scale );
}

TransformNode* Scene::LoadAssimp( const char* _File, const float _Scale )
{
	TransformNode* root = new TransformNode();
	Mesh* current = 0;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile( _File, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs );
	for (uint i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		current = new Mesh();
		current->CreateFromAssimp( mesh );
	}
	return 0;
}

TransformNode* Scene::LoadBIN( const char* _File, const float _Scale )
{
	TransformNode* root = new TransformNode();
	FILE* file = fopen( _File, "rb" );
	int mtlNameLength;
	char temp[1024];
	fread( &mtlNameLength, 1, 4, file );
	fread( temp, 1, mtlNameLength, file );
	LoadMTL( temp );
	while (1)
	{
		vector<vec3> vlist, nlist;
		vector<vec2> uvlist;
		vector<uint> index;
		int vertCount, normalCount, uvCount, indexCount, matNameLength;
		fread( &vertCount, 1, 4, file );
		if (vertCount == 0) break;
		vlist.resize( vertCount );
		fread( &vlist[0], sizeof( vlist[0] ), vertCount, file );
		fread( &normalCount, 1, 4, file );
		nlist.resize( normalCount );
		fread( &nlist[0], sizeof( nlist[0] ), normalCount, file );
		fread( &uvCount, 1, 4, file );
		uvlist.resize( uvCount );
		fread( &uvlist[0], sizeof( uvlist[0] ), uvCount, file );
		fread( &indexCount, 1, 4, file );
		index.resize( indexCount );
		fread( &index[0], sizeof( index[0] ), indexCount, file );
		fread( &matNameLength, 1, 4, file );
		fread( temp, 1, matNameLength, file );
		Material* mat = FindMaterial( temp );
		Mesh* current = new Mesh();
		current->SetMaterial( mat );
		root->AddChild( current );
		for (size_t s = vlist.size(), i = 0; i < s; i++) vlist[i] *= _Scale;
		current->CreateFromArrays( vlist, nlist, uvlist, index );
	}
	fclose( file );
	return root;
}

TransformNode* Scene::LoadOBJ( const char* _File, const float _Scale )
{
	// obj file loader: converts indexed obj file into indexed multi-mesh ogl node
	struct UniqueVertex
	{
		UniqueVertex( int v, int n = -1, int t = -1 ) : vertex( v ), normal( n ), uv( t ), next( -1 ), subid( 0 ) {}
		int vertex, normal, uv, next, subid, idx;
	};
	TransformNode* root = new TransformNode(), * group = root;
	Mesh* current = 0, * nextMesh = 0;
	char binFile[1024];
	strcpy( binFile, _File );
	*(strstr( binFile, ".obj" )) = 0;
	char rootName[1024], * pos = binFile;
	while (strstr( pos, "/" )) pos = strstr( pos, "/" ) + 1;
	while (strstr( pos, "\\" )) pos = strstr( pos, "\\" ) + 1;
	strcpy( rootName, pos );
	strcat( rootName, "_root" );
	root->SetName( rootName );
	strcat( binFile, ".bin" );
	FILE* file = fopen( _File, "r" ), * bin = fopen( binFile, "wb" );
	// int nameLength = strlen( rootName ) + 1;
	// fwrite( &nameLength, 1, 4, bin );
	// fwrite( rootName, 1, nameLength, bin );
	if (file)
	{
		vector<vec3> vlist, nlist, vlist_, nlist_;
		vector<vec2> uvlist, uvlist_;
		vector<uint> index, index_;
		vector<UniqueVertex> unique;
		vlist.reserve( 100000 );
		nlist.reserve( 100000 );
		unique.reserve( 100000 );
		int subID = 1, formata;
		while (!feof( file ))
		{
			current = nextMesh, subID++, formata = -1;
			bool hasUV = false;
			char line[2048], tmp[2048];
			while (!feof( file ))
			{
				line[0] = 0;
				fgets( line, 1023, file );
				if (!_strnicmp( line, "mtllib", 6 ))
				{
					sscanf( line + 7, "%s", tmp );
					LoadMTL( tmp );
					int mtlNameLength = (int)strlen( tmp ) + 1;
					fwrite( &mtlNameLength, 1, 4, bin );
					fwrite( tmp, 1, mtlNameLength, bin );
					formata = -1, hasUV = false;
				}
				if (!_strnicmp( line, "usemtl", 6 ))
				{
					// prepare new mesh
					sscanf( line + 7, "%s", tmp );
					nextMesh = new Mesh();
					Material* mat = FindMaterial( tmp );
					nextMesh->SetMaterial( mat );
					group->AddChild( nextMesh );
					subID++;
					break;
				}
				if (line[0] == 'g')
				{
					formata = -1;
					char* g = line + 2;
					while ((g[0]) && (g[strlen( g ) - 1] < 33)) g[strlen( g ) - 1] = 0;
					TransformNode* newGroup = new TransformNode();
					newGroup->SetName( g );
					root->AddChild( newGroup );
					group = newGroup;
				}
				if (line[0] == 'v')
				{
					if (line[1] == ' ')
					{
						vec3 vertex;
						sscanf( line + 2, "%f %f %f", &vertex.x, &vertex.y, &vertex.z );
						vlist.push_back( vertex * _Scale );
						unique.push_back( UniqueVertex( (int)vlist.size() - 1 ) );
					}
					else if (line[1] == 't') { vec2 uv; sscanf( line + 3, "%f %f", &uv.x, &uv.y ); uv.y = 1.0f - uv.y; uvlist.push_back( uv ); }
					else if (line[1] == 'n') { vec3 normal; sscanf( line + 3, "%f %f %f", &normal.x, &normal.y, &normal.z ); nlist.push_back( normal ); }
				}
				if (line[0] != 'f') continue;
				if (formata == -1)
				{
					formata = 0;
					for (int i = 0; i < (int)strlen( line ); i++) if (line[i] == '/' && line[i + 1] == '/') formata = 1;
				}
				int v[3], n[3], t[3] = { 0, 0, 0 };
				if (formata) sscanf( line + 2, "%i//%i %i//%i %i//%i", &v[0], &n[0], &v[1], &n[1], &v[2], &n[2] );
				sscanf( line + 2, "%i/%i/%i %i/%i/%i %i/%i/%i", &v[0], &t[0], &n[0], &v[1], &t[1], &n[1], &v[2], &t[2], &n[2] );
				for (int i = 0; i < 3; i++)
				{
					int vidx = v[i] - 1, idx = vidx, lastIdx = idx, nidx = n[i] - 1, uvidx = t[i] - 1;
					if (uvidx > -1) hasUV = true;
					do
					{
						UniqueVertex& u = unique[idx];
						if (u.subid != subID) // vertex not used before by this mesh
						{
							u.subid = subID, u.next = -1, u.vertex = vidx, u.normal = nidx, u.uv = uvidx;
							index.push_back( idx );
							break;
						}
						else if ((u.normal == nidx) && (u.uv == uvidx)) // vertex used before, but the same
						{
							index.push_back( idx );
							break;
						}
						lastIdx = idx, idx = u.next;
					} while (idx > -1);
					if (idx == -1) // need new entry for the current vertex
					{
						uint newIdx = (uint)unique.size();
						unique[lastIdx].next = newIdx;
						index.push_back( newIdx );
						unique.push_back( UniqueVertex( vidx, nidx, uvidx ) );
						unique[newIdx].subid = subID;
					}
				}
			}
			if (current)
			{
				subID++;
				vlist_.clear();
				nlist_.clear();
				uvlist_.clear();
				index_.clear();
				for (uint i = 0; i < index.size(); i++)
				{
					UniqueVertex& u = unique[index[i]];
					if (u.subid == subID) index_.push_back( u.idx ); else // first time we encounter this UniqueVertex, emit
					{
						vlist_.push_back( vlist[u.vertex] );
						nlist_.push_back( nlist[u.normal] );
						if (hasUV) uvlist_.push_back( uvlist[u.uv] );
						index_.push_back( (int)vlist_.size() - 1 );
						u.idx = (int)vlist_.size() - 1, u.subid = subID;
					}
				}
				// write to binary file
				int vertCount = (int)vlist_.size();
				fwrite( &vertCount, 1, 4, bin );
				fwrite( &vlist_[0], sizeof( vlist_[0] ), vlist_.size(), bin );
				int normalCount = (int)nlist_.size();
				fwrite( &normalCount, 1, 4, bin );
				fwrite( &nlist_[0], sizeof( nlist_[0] ), nlist_.size(), bin );
				int uvCount = (int)uvlist_.size();
				fwrite( &uvCount, 1, 4, bin );
				if (uvCount > 0) fwrite( &uvlist_[0], sizeof( uvlist_[0] ), uvlist_.size(), bin );
				int indexCount = (int)index_.size();
				fwrite( &indexCount, 1, 4, bin );
				fwrite( &index_[0], sizeof( index_[0] ), index_.size(), bin );
				int matNameLength = (int)strlen( current->GetMaterial()->GetName() ) + 1;
				fwrite( &matNameLength, 1, 4, bin );
				fwrite( current->GetMaterial()->GetName(), 1, matNameLength, bin );
				// create mesh
				current->CreateFromArrays( vlist_, nlist_, uvlist_, index_ );
				// clean up
				while (unique.size() > vlist.size()) unique.pop_back();
				index.clear();
			}
		}
		fclose( file );
		int zero = 0;
		fwrite( &zero, 1, 4, bin );
		fclose( bin );
	}
	return root;
}

void Scene::LoadMTL( const char* _File )
{
	char fname[1024], line[1024], cmd[256];
	strcpy( fname, scenePath );
	strcat( fname, "/" );
	strcat( fname, _File );
	FILE* file = fopen( fname, "r" );
	if (!file) return;
	Material* current = 0;
	int firstIdx = (int)matList.size();
	while (!feof( file ))
	{
		line[0] = 0;
		fgets( line, 1022, file );
		if (line[0]) if (line[strlen( line ) - 1] < 32) line[strlen( line ) - 1] = 0; // clean '10' at end
		sscanf( line, "%s", cmd );
		vec3 rgb;
	#ifndef NOALPHA
		if (!_stricmp( cmd, "alpha" )) if (current->GetTexture()) current->GetTexture()->SetAlpha( true );
	#endif
		if (!_stricmp( cmd, "newmtl" ))
		{
			char matName[128];
			sscanf( line + strlen( cmd ), "%s", matName );
			current = new Material();
			current->SetName( matName );
			matList.push_back( current );
		}
		if (!_stricmp( cmd, "Kd" ))
		{
			sscanf( line + 3, "%f %f %f", &rgb.x, &rgb.y, &rgb.z );
			current->SetColor( rgb );
		}
		if ((!_stricmp( cmd, "map_Kd" )) || (!_stricmp( cmd, "map_bump" )) || (!_stricmp( cmd, "map_env" )))
		{
			bool bump = (_stricmp( cmd, "map_bump" ) == 0);
			bool env = (_stricmp( cmd, "map_env" ) == 0);
			char fname[256], * pos = strstr( line, " " );
			if (!pos) continue; // huh
			char* tname = pos + 1;
			if (strstr( pos + 1, "textures\\" )) tname = strstr( pos + 1, "textures\\" ) + 9;
			if (tname[0])
			{
				Texture* texture = FindTexture( tname );
				if (!texture)
				{
					strcpy( fname, scenePath );
					strcat( fname, "textures/" );
					strcat( fname, tname );
					if (env) texture = skyCubeMap; else
					{
						texture = new Texture();
						texture->Load( fname );
					}
					texture->SetName( tname );
					texList.push_back( texture );
				}
				if (bump) current->SetNormalMap( texture );
				else if (env) current->SetEnvMap( texture );
				else current->SetTexture( texture );
			}
		}
	}
	fclose( file );
	// generate shaders for the materials
	for (size_t s = matList.size(), i = firstIdx; i < s; i++)
	{
		Material* mat = matList[i];
		mat->CreateShaders();
	}
}

void Scene::UpdateLights()
{
	Renderer::statics.lightCount = (int)lightList.size();
	for (size_t s = lightList.size(), i = 0; i < s; i++)
	{
		Renderer::statics.light[i].pos = lightList[i]->position;
		Renderer::statics.light[i].col = lightList[i]->color;
		if (lightList[i]->node) lightList[i]->node->SetTransform( translate( mat4( 1 ), lightList[i]->position ) );
	}
}

struct sortClass { bool operator() ( DrawCall a, DrawCall b ) { return (a.key < b.key); } } sorter;

void Scene::Render( RenderType _Type, mat4& _World, mat4& _View, mat4& _Light )
{
	// prepare frame rendering
	Node::SetScene( this );
	Material::currentShader = 0;
	// go
#ifdef SORTEDRENDERING
	Renderer::drawCall.clear();
	Renderer::drawTransform.clear();
	for (uint s = nodeList.size(), i = 0; i < s; i++)
	{
		DrawTransform dt;
		dt.world = dt.view = mat4( 1.0 );
		Renderer::drawTransform.push_back( dt );
		nodeList[i]->Gather( _Type, Renderer::drawTransform.size() - 1, _World, _View, _Light );
	}
	// std::sort( Renderer::drawCall.begin(), Renderer::drawCall.end(), sorter ); // does not help!?
	uint lastTransform = 999999, lastKey = 0;
	for (uint s = Renderer::drawCall.size(), i = 0; i < s; i++)
	{
		DrawCall& dc = Renderer::drawCall[i];
		if (dc.transform != lastTransform)
		{
			DrawTransform& dt = Renderer::drawTransform[dc.transform];
			Renderer::statics.view = dt.view;
			Renderer::statics.world = dt.world;
			Renderer::statics.transformUBO->Update();
			lastTransform = dc.transform;
		}
		glBindVertexArray( dc.mesh->GetID() );
		if (dc.key != lastKey)
		{
			dc.mesh->GetMaterial()->Bind( _Type );
			lastKey = dc.key;
		}
		glDrawElements( GL_TRIANGLES, dc.mesh->GetTriCount() * 3, GL_UNSIGNED_INT, 0 );
	}
#else
	for (size_t s = nodeList.size(), i = 0; i < s; i++) nodeList[i]->Render( _Type, _World, _View, _Light );
#endif
}

void Scene::RenderShadow( mat4& _View )
{
	for (size_t s = nodeList.size(), i = 0; i < s; i++) nodeList[i]->RenderShadow( _View );
}

void Scene::RenderShadowCube( mat4& _World )
{
	for (size_t s = nodeList.size(), i = 0; i < s; i++) nodeList[i]->RenderShadowCube( _World );
}

// EOF