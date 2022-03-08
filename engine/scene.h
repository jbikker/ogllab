#pragma once

namespace ogllab {

class Scene
{
public:
	// constructor / destructor
	Scene();
	~Scene();
	// get / set
	char* GetScenePath() { return scenePath; }
	vector<PointLight*>& GetLightList() { return lightList; }
	uint GetLightCount() { return (int)lightList.size(); }
	// methods
	TransformNode* Load( const char* _File, const float _Scale );
	void LoadMTL( const char* _File );
	void UpdateLights();
	void Render( RenderType _Type, mat4& _World, mat4& _View, mat4& _Light );
	void RenderShadow( mat4& _View );
	void RenderShadowCube( mat4& _World );
	void ExtractScenePath( const char* _File );
	Material* FindMaterial( const char* _Name );
	Texture* FindTexture( const char* _Name );
	Node* FindNode( const char* _Name );
	PointLight* GetLight( const uint _Idx ) { return lightList[_Idx]; }
	// private methods
private:
	TransformNode* LoadAssimp( const char* _File, const float _Scale );
	TransformNode* LoadOBJ( const char* _File, const float _Scale );
	TransformNode* LoadBIN( const char* _File, const float _Scale );
	// data members
private:
	vector<Node*> nodeList;
	vector<Material*> matList;
	vector<Texture*> texList;
	vector<PointLight*> lightList;
	char* scenePath;
	Texture* skyCubeMap;
};

}

// EOF