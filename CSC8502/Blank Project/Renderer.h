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

	void DrawNode(SceneNode* n, Shader*);

protected:
	//make a new texture...
	void GenerateScreenTexture(GLuint& into, bool depth = false);
	void BuildNodeLists(SceneNode* from);
	void DrawSkybox();
	void DrawCar();
	void DrawRain();
	void DrawTransparentNode();
	void DrawHeightmap();
	void DrawWater();
	void DrawTree();
	void DrawCube();
	void DrawTreePot();
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();

	//Shader* sceneShader;		//Shader to fill GBuffers
	Shader* spinShader;
	Shader* lightShader;
	Shader* bumpShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* basicSceneShader;
	Shader* sceneShader;
	Shader* shadowShader;
	Shader* blendShader;
	Shader* combineShader;

	vector<SceneNode*> buildings;
	SceneNode* transparentQuad;
	HeightMap* heightMap;		//Terrain
	SceneNode* tree;
	SceneNode* treePot;
	SceneNode* dogeQuad;
	Light* light;
	Light* billBoardLight;
	Light* billBoardLight2;
	Light* characterLight;
	SceneNode* spinCube;
	SceneNode* blendCube;
	Mesh* sphere;				//Light volume
	Mesh* quad;					//To draw a full-screen quad
	Camera* camera;				//Our usual camera
	SceneNode* car;
	SceneNode* car2;
	GLuint earthTex;
	GLuint earthBump;
	Mesh* carMesh;
	//vector<SceneNode> rain;
	bool noCar;
	bool rotatedCar;
	float deleteTimer;

	//scene graph stuff
	Shader* nodeShader;
	GLuint nodeTexture;
	GLuint nodeBump;
	Frustum frameFrustum;

	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
	SceneNode* root;
	SceneNode* water;
	Mesh* cube;
	SceneNode* cubeRobot;

	//model animation
	SceneNode* girlNode;
	Shader* animShader;
	MeshAnimation* anim;
	MeshMaterial* material;
	vector<GLuint> matTextures;

	//cube mapping stuff
	GLuint cubeMap;
	GLuint waterTex;
	float waterRotate;
	float waterCycle;
	Mesh* skyboxQuad;

	//shadow mapping stuff
	void DrawShadowScene();
	void DrawShadowedObjects();
	void DrawShadowNode(SceneNode* n, Matrix4 sceneTransform, Shader* passedShader);

	vector<Mesh*> sceneMeshes;
	vector<Matrix4> sceneTransforms;
	GLuint shadowTex;
	GLuint shadowFBO;

	GLuint sceneDiffuse;
	GLuint sceneBump;
	float sceneTime;
	Light* shadowLight;
	Light* shadowLight2;
	bool lightDir = false;
};
