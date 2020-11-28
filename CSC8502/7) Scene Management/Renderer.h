#pragma once

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"
class HeightMap;
class Camera;
class SceneNode;
class Mesh;
class Shader;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void UpdateScene(float msec) override;
	void RenderScene() override;

protected:
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);

	void FillBuffers();			//G-Buffer fill render pass
	void DrawPointLights();		//Lighting render pass
	void CombineBuffers();		//Combination render pass
	//make a new texture...
	void GenerateScreenTexture(GLuint& into, bool depth = false);

	SceneNode* root;
	Camera* camera;
	Mesh* quad;
	Mesh* cube;
	SceneNode* car;
	Shader* shader;
	GLuint texture;
	GLuint earthTex;
	GLuint earthBump;
	HeightMap* heightMap;		//Terrain
	Frustum frameFrustum;
	Mesh* sphere;				//Light volume

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

	Light* pointLights;			//Array of lighting data

	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
};