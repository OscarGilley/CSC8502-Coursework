#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"
class SceneNode;
class CubeRobot;
class Camera;
class Mesh;
class HeightMap;
class MeshAnimation;
class MeshMaterial;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;

	void DrawNode(SceneNode* n);

protected:
	void FillBuffers();			//G-Buffer fill render pass
	void DrawPointLights();		//Lighting render pass
	void CombineBuffers();		//Combination render pass
	//make a new texture...
	void GenerateScreenTexture(GLuint& into, bool depth = false);
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();

	Shader* sceneShader;		//Shader to fill GBuffers
	Shader* pointlightShader;	//Shader to calc lighting
	Shader* combineShader;

	GLuint bufferFBO;			//FBO for our G-Buffer pass
	GLuint bufferColourTex;		//Albedo goes here
	GLuint bufferNormalTex;		//Normals go here
	GLuint bufferDepthTex;		//Depth goes here

	GLuint pointLightFBO;		//FBO for our lighting pass
	GLuint lightDiffuseTex;		//Store diffuse lighting
	GLuint lightSpecularTex;	//Store specular lighting

	HeightMap* heightMap;		//Terrain
	Light* pointLights;			//Array of lighting data
	Light* light;
	Mesh* sphere;				//Light volume
	Mesh* quad;					//To draw a full-screen quad
	Camera* camera;				//Our usual camera
	SceneNode* car;
	GLuint earthTex;
	GLuint earthBump;

	//scene graph stuff
	Shader* nodeShader;
	GLuint nodeTexture;
	Frustum frameFrustum;

	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
	SceneNode* root;
	Mesh* cube;
	SceneNode* cubeRobot;

	//model animation
	SceneNode* girlNode;
	Shader* shader;
	MeshAnimation* anim;
	MeshMaterial* material;
	vector<GLuint> matTextures;

	int currentFrame;
	float frameTime;

};
