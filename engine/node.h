#pragma once

namespace ogllab {

class Scene;
class Node
{
public:
	// constructor / destructor
	Node();
	// get / set
	Node* GetChild( uint _Idx ) { return child[_Idx]; }
	void AddChild( Node* _Child );
	int ChildCount() { return (int)child.size(); }
	char* GetName() { return name; }
	void SetName( const char* _Name );
	Node* Find( const char* _Name );
	void SetFlags( NodeFlags _Flags );
	NodeFlags GetFlags() { return flags; }
	// methods
	virtual void Render( const RenderType _Type, mat4& _World, mat4& _View, mat4& _Light ) = 0;
	virtual void Gather( const RenderType _Type, uint _Transform, mat4& _World, mat4& _View, mat4& _Light ) = 0;
	virtual void RenderShadow( mat4& _View ) = 0;
	virtual void RenderShadowCube( mat4& _World ) = 0;
	virtual void GatherTriangles( vector<Triangle>& _Tri, vector<Material*>& _Mat ) = 0;
	// static methods
	static void SetScene( Scene* _Scene ) { scene = _Scene; }
	// data members
protected:
	vector<Node*> child;
	AABB bounds;
	static Scene* scene;
	char* name;
	NodeFlags flags;
};

class TransformNode : public Node
{
public:
	// constructor / destructor
	TransformNode();
	// get / set
	void SetTransform( mat4& _Transform ) { transform = _Transform; }
	mat4& GetTransform() { return transform; }
	// methods
	void Render( const RenderType _Type, mat4& _World, mat4& _View, mat4& _Light );
	void Gather( const RenderType _Type, uint _Transform, mat4& _World, mat4& _View, mat4& _Light );
	void RenderShadow( mat4& _View );
	void RenderShadowCube( mat4& _World );
	void GatherTriangles( vector<Triangle>& _Tri, vector<Material*>& _Mat );
	// data members
private:
	mat4 transform;
};

class MeshBase : public Node
{
public:
	// constructor / destructor
	MeshBase();
	~MeshBase();
	// get / set
	void SetMaterial( Material* _Mat ) { material = _Mat; }
	Material* GetMaterial() { return material; }
	GLuint GetID() { return vao; }
	uint GetTriCount() { return tris; }
	// methods
	GLuint CreateVBO( const GLfloat* _Data, const uint _Size );
	void BindVBO( const uint _Idx, const uint _Elements, const GLuint _ID );
	void CalculateKey( const RenderType _Type );
	virtual void Render( const RenderType _Type, mat4& _World, mat4& _View, mat4& _Light );
	virtual void Gather( const RenderType _Type, uint _Transform, mat4& _World, mat4& _View, mat4& _Light );
	virtual void RenderShadow( mat4& _View );
	virtual void RenderShadowCube( mat4& _World );
	void GatherTriangles( vector<Triangle>& _Tri, vector<Material*>& _Mat );
	// data members
protected:
	Material* material;
	GLuint vao;
	GLuint vertexBuffer, UVBuffer, normalBuffer, indexBuffer;
	vector<vec3> vertices, normals;
	vector<vec2> uvs;
	vector<uint> indices;
	uint tris;
	uint key;
};

class Quad : public MeshBase
{
public:
	// constructor / destructor
	Quad();
	// methods
	void Render();
};

class Mesh : public MeshBase
{
public:
	// constructor / destructor
	Mesh() : angle( 0 ) {}
	// methods
	void CreateFromArrays( vector<vec3>& _Verts, vector<vec3>& _Normals, vector<vec2>& _UVs, vector<uint>& _Indices );
	void CreateFromAssimp( const aiMesh* _Mesh );
	void CreateDOFGrid( const ivec2 _Res );
	// data members
private:
	float angle;
};

class SkyBox : public MeshBase
{
public:
	// constructor / destructor
	SkyBox();
	~SkyBox();
	// methods
	void Render( const RenderType _Type, mat4& _World, mat4& _View, mat4& _Light );
	void Gather( const RenderType _Type, uint _Transform, mat4& _World, mat4& _View, mat4& _Light );
	void RenderShadow( mat4& _View ) { /* don't draw skybox for shadow mapping */ }
	void RenderShadowCube( mat4& _World ) { /* don't draw skybox for shadow mapping */ }
	// data members
private:
	float angle;
};

}

// EOF