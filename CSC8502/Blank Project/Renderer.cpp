#include "Renderer.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Camera.h"
#include "../nclgl/Light.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/CubeRobot.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include <algorithm>

#define SHADOWSIZE 2048

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	sphere = Mesh::LoadFromMeshFile("Sphere.msh");
	quad = Mesh::GenerateQuad();
	heightMap = new HeightMap(TEXTUREDIR"calmnoise2.png");

	nodeTexture = SOIL_load_OGL_texture(TEXTUREDIR"Rock_04_DIFF.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	nodeBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	GLuint doge = SOIL_load_OGL_texture(TEXTUREDIR"doge.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	GLuint angryDoge = SOIL_load_OGL_texture(TEXTUREDIR"angerdoge.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	GLuint upsideDownDoge = SOIL_load_OGL_texture(TEXTUREDIR"upsidedowndoge.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	GLuint dogeBump = SOIL_load_OGL_texture(TEXTUREDIR"dogemap2.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Rock_04_DIFF.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Rock_04_NRM.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);

	nodeShader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	animShader = new Shader("PerPixelVertAnim.glsl", "PerPixelFragAnim.glsl");

	Vector3 heightmapSize = heightMap->GetHeightmapSize();

	camera = new Camera(-45.0f, 0.0f, Vector3(0.0f, 1275.0f, 0.0f));

	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(0.2f, 0.2f, 1.0f, 1.5f), heightmapSize.x * 1.0f);
	//light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1.0f, 1.0f, 1.0f, 1.0f), heightmapSize.x * 1.5f); //alternative light!

	basicSceneShader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
	bumpShader = new Shader("bumpVertex.glsl", "bumpFragment.glsl");
	blendShader = new Shader("PerPixelVertTwoTex.glsl", "PerPixelFragTwoTex.glsl");
	combineShader = new Shader("PerPixelVertTwoTexBlend.glsl", "PerPixelFragTwoTexBlend.glsl");

	sceneShader = new Shader("shadowSceneVert.glsl", "shadowSceneFrag.glsl");
	shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");

	if (!nodeShader->LoadSuccess() || !animShader->LoadSuccess() || !basicSceneShader->LoadSuccess() || !reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess()
		|| !lightShader->LoadSuccess() || !bumpShader->LoadSuccess() || !blendShader->LoadSuccess() || !combineShader->LoadSuccess() || !sceneShader->LoadSuccess()
		|| !shadowShader->LoadSuccess()) {
		return;
	}

	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"lagoon_west.bmp", TEXTUREDIR"lagoon_east.bmp", TEXTUREDIR"lagoon_up.bmp",
		TEXTUREDIR"lagoon_down.bmp", TEXTUREDIR"lagoon_south.bmp", TEXTUREDIR"lagoon_north.bmp", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	waterRotate = 0.0f;
	waterCycle = 0.0f;
	
	cube = Mesh::LoadFromMeshFile("OffsetCubeY.msh");

	root = new SceneNode();

	root->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	root->SetBoundingRadius(409600.0f);
	root->SetMesh(heightMap);
	root->SetTexture(earthTex);
	root->SetBump(earthBump);

	//cuberobot is the center of the heightmap, used for easy translations about that point.
	cubeRobot = new SceneNode(cube);
	cubeRobot->SetTransform(root->GetWorldTransform() * Matrix4::Translation(Vector3(heightmapSize.x / 2, heightmapSize.y, heightmapSize.z / 2)));
	cubeRobot->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	cubeRobot->SetMesh(cube);
	cubeRobot->SetTexture(earthTex);
	cubeRobot->SetModelScale(Vector3(0, 0, 0));
	cubeRobot->SetBoundingRadius(100000.0f);
	root->AddChild(cubeRobot);

	//makes buildings
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 4; j++) {
			SceneNode* building = new SceneNode(Mesh::LoadFromMeshFile("Highrise_8.msh"));
			building->SetTexture(nodeTexture);
			building->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
			building->SetBoundingRadius(4096000.0f);
			building->SetLight(light);
			building->SetTransform(cubeRobot->GetTransform() * Matrix4::Translation(Vector3(500 + (i * 500), -125, -1000 + (j * 300))));
			building->SetModelScale(Vector3(100, 400, 100));
			root->AddChild(building);
			buildings.push_back(building);
		}
	}

	billBoardLight = new Light((Vector3(-1100, 500, -1600)), Vector4(0.631f, 0.09f, 0.949f, 1.0f), heightmapSize.x * 2);

	dogeQuad = new SceneNode(Mesh::GenerateQuad());
	dogeQuad->SetTransform(cubeRobot->GetTransform() * Matrix4::Rotation(30.0f, Vector3(0, 1, 0)) * Matrix4::Translation(Vector3(-1000, 500, -1500)));
	dogeQuad->SetColour(Vector4(0.631f, 0.09f, 0.949f, 0.7f));
	dogeQuad->SetTexture(upsideDownDoge);
	dogeQuad->SetLight(billBoardLight);
	dogeQuad->SetModelScale(Vector3(345, 95, 10));
	dogeQuad->SetBoundingRadius(100000.0f);
	root->AddChild(dogeQuad);

	for (int k = 0; k < 2; k++) {
		SceneNode* c = new SceneNode(Mesh::LoadFromMeshFile("Cylinder.msh"));
		c->SetTransform(dogeQuad->GetWorldTransform() * Matrix4::Translation(Vector3(-100 + k * 200, -350, 0)));
		c->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		c->SetTexture(earthTex);
		c->SetSecondTexture(doge);
		c->SetLight(billBoardLight);
		c->SetModelScale(Vector3(10, 250, 10));
		c->SetBoundingRadius(100000.0f);
		dogeQuad->AddChild(c);
	}

	SceneNode* cy = new SceneNode(Mesh::LoadFromMeshFile("Cylinder.msh"));
	cy->SetTransform(Matrix4::Rotation(90.0f, Vector3(0, 0, 1)) * dogeQuad->GetWorldTransform() * Matrix4::Translation(Vector3(100, 0, 0)));
	cy->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	cy->SetTexture(earthTex);
	cy->SetLight(billBoardLight);
	cy->SetModelScale(Vector3(10, 350, 10));
	cy->SetBoundingRadius(100000.0f);
	dogeQuad->AddChild(cy);

	SceneNode* cy2 = new SceneNode(Mesh::LoadFromMeshFile("Cylinder.msh"));
	cy2->SetTransform(Matrix4::Rotation(90.0f, Vector3(0, 0, 1)) * dogeQuad->GetWorldTransform() * Matrix4::Translation(Vector3(-100, 0, 0)));
	cy2->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	cy2->SetTexture(earthTex);
	cy2->SetLight(billBoardLight);
	cy2->SetModelScale(Vector3(10, 350, 10));
	cy2->SetBoundingRadius(100000.0f);
	dogeQuad->AddChild(cy2);

	SceneNode* cy3 = new SceneNode(Mesh::LoadFromMeshFile("Cylinder.msh"));
	cy3->SetTransform(dogeQuad->GetWorldTransform() * Matrix4::Translation(Vector3(-350, 0, 0)));
	cy3->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	cy3->SetTexture(earthTex);
	cy3->SetLight(billBoardLight);
	cy3->SetModelScale(Vector3(10, 100, 10));
	cy3->SetBoundingRadius(100000.0f);
	dogeQuad->AddChild(cy3);

	SceneNode* cy4 = new SceneNode(Mesh::LoadFromMeshFile("Cylinder.msh"));
	cy4->SetTransform(dogeQuad->GetWorldTransform() * Matrix4::Translation(Vector3(350, 0, 0)));
	cy4->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	cy4->SetTexture(earthTex);
	cy4->SetLight(billBoardLight);
	cy4->SetModelScale(Vector3(10, 100, 10));
	cy4->SetBoundingRadius(100000.0f);
	dogeQuad->AddChild(cy4);

	blendCube = new SceneNode(Mesh::LoadFromMeshFile("OffsetCubeY.msh"));
	blendCube->SetTransform(cubeRobot->GetTransform() * Matrix4::Rotation(30.0f, Vector3(0, 1, 0)) * Matrix4::Translation(Vector3(1000, 400, -1500)));
	blendCube->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	blendCube->SetTexture(doge);
	blendCube->SetSecondTexture(angryDoge);
	blendCube->SetLight(light);
	blendCube->SetModelScale(Vector3(50, 50, 50));
	blendCube->SetBoundingRadius(100000.0f);

	SceneNode* pole = new SceneNode(Mesh::LoadFromMeshFile("Cylinder.msh"));
	pole->SetTransform(cubeRobot->GetTransform() * Matrix4::Rotation(30.0f, Vector3(0, 1, 0)) * Matrix4::Translation(Vector3(1000, 135, -1500)));
	pole->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	pole->SetTexture(earthTex);
	pole->SetLight(light);
	pole->SetModelScale(Vector3(10, 250, 10));
	pole->SetBoundingRadius(100000.0f);
	root->AddChild(pole);


	tree = new SceneNode(Mesh::LoadFromMeshFile("green_leaf_tree.msh"));
	tree->SetMaterial(new MeshMaterial("green_leaf_tree.mat"));
	tree->SetMatTextures();
	tree->SetLight(light);
	tree->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	tree->SetBoundingRadius(4000000.0f);
	tree->SetTransform(cubeRobot->GetTransform() * Matrix4::Translation(Vector3(-100, -700, -100)));
	tree->SetModelScale(Vector3(100, 100, 100));
	root->AddChild(tree);

	treePot = new SceneNode(Mesh::LoadFromMeshFile("OffsetCubeY.msh"));
	treePot->SetTransform(tree->GetWorldTransform() * Matrix4::Translation(Vector3(2618, 123, 715)));
	treePot->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	treePot->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"Dirt_DIFF.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	treePot->SetSecondTexture(SOIL_load_OGL_texture(TEXTUREDIR"Dirty_Grass_DIFF.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	treePot->SetLight(light);
	treePot->SetModelScale(Vector3(10, 10, 10));
	treePot->SetBoundingRadius(100000.0f);

	car2 = new SceneNode(Mesh::LoadFromMeshFile("retro.msh"));
	car2->SetMaterial(new MeshMaterial("retro.mat"));
	car2->SetMatTextures();
	car2->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	car2->SetLight(light);
	car2->SetBoundingRadius(4096000.0f);
	car2->SetTransform(cubeRobot->GetTransform() * Matrix4::Translation(Vector3(-1250, -120, 0)));
	car2->SetModelScale(Vector3(50, 50, 50));
	root->AddChild(car2);
	

	characterLight = new Light(heightmapSize * Vector3(0.8f, 1.0f, 0.8f), Vector4(1.0f, 0.8f, 0.8f, 1.0f), heightmapSize.x * 0.5f);
	
	girlNode = new SceneNode(Mesh::LoadFromMeshFile("eve_j_gonzales.msh"));
	girlNode->SetAnimation(new MeshAnimation("eve_j_gonzales.anm"));
	girlNode->SetMaterial(new MeshMaterial("eve_j_gonzales.mat"));
	girlNode->SetMatTextures();
	girlNode->SetLight(characterLight);
	girlNode->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	girlNode->SetBoundingRadius(4096000.0f);
	girlNode->SetModelScale(Vector3(100, 100, 100));
	girlNode->SetTransform(cubeRobot->GetTransform() * Matrix4::Translation(Vector3(1000, -125, 1000)));
	root->AddChild(girlNode);
	
	shadowLight = new Light(Vector3(70.0f, 240.0f,70.0f), Vector4(1, 1, 1, 1), 500.0f);
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	
	sceneMeshes.emplace_back(Mesh::GenerateQuad());
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("cube.msh")); //billboard
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Cylinder.msh")); //base pole
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Cylinder.msh")); //frame
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Cylinder.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Cylinder.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Cylinder.msh"));
	//sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Car2.msh"));
	
	sceneDiffuse = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sceneBump = SOIL_load_OGL_texture(TEXTUREDIR"dogemap2.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	SetTextureRepeating(sceneDiffuse, true);
	SetTextureRepeating(sceneBump, true);
	sceneTime = 0.0f;

	sceneTransforms.resize(7);
	//sceneTransforms.resize(8);
	sceneTransforms[0] = Matrix4::Translation(Vector3(100, 230, 100)) * Matrix4::Scale(Vector3(100, 100, 100)) * Matrix4::Rotation(90, Vector3(1, 0, 0));
	sceneTransforms[1] = Matrix4::Translation(Vector3(100, 240, 100)) * Matrix4::Scale(Vector3(30, 10, 1));
	sceneTransforms[2] = Matrix4::Translation(Vector3(100, 225, 100)) * Matrix4::Scale(Vector3(1, 10, 1));
	sceneTransforms[3] = Matrix4::Translation(Vector3(115, 240, 100)) * Matrix4::Scale(Vector3(1, 5, 1)); 
	sceneTransforms[4] = Matrix4::Translation(Vector3(85, 240, 100)) * Matrix4::Scale(Vector3(1, 5, 1));
	sceneTransforms[5] = Matrix4::Translation(Vector3(100, 245, 100)) * Matrix4::Scale(Vector3(15, 1, 1)) * Matrix4::Rotation(90.0f, Vector3(0, 0, 1));
	sceneTransforms[6] = Matrix4::Translation(Vector3(100, 235, 100)) * Matrix4::Scale(Vector3(15, 1, 1)) * Matrix4::Rotation(90.0f, Vector3(0, 0, 1));
	//sceneTransforms[7] = Matrix4::Translation(Vector3(40, 215, 100)) * Matrix4::Scale(Vector3(5, 5, 5));
	
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	sceneTime = 0.0f;

	car = new SceneNode(cube);
	car->SetTransform(cubeRobot->GetTransform()* Matrix4::Translation(Vector3(-200, -260, -700)));
	car->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	car->SetMesh(Mesh::LoadFromMeshFile("Car2.msh"));
	car->SetTexture(earthTex);
	car->SetModelScale(Vector3(30, 30, 30));
	car->SetBoundingRadius(100000.0f);
	noCar = false;

	carMesh = Mesh::LoadFromMeshFile("Car2.msh");
	
	rotatedCar = false;
	deleteTimer = 0.0f;

	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	
	

	init = true;
}

Renderer::~Renderer(void) {
	delete nodeShader;

	delete heightMap;
	delete cubeRobot;
	delete cube;
	delete camera;
	delete sphere;
	delete quad;
	delete light;
	delete tree;
	

	for (int i = 0; i < buildings.size(); i++) {
		delete buildings.at(i);
	}

	for (int i = 0; i < sceneMeshes.size(); i++) {
		delete sceneMeshes.at(i);
	}

	
}

void Renderer::GenerateScreenTexture(GLuint& into, bool depth) {
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH_COMPONENT24 : GL_RGBA8;
	GLuint type = depth ? GL_DEPTH_COMPONENT : GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, type, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	waterRotate += dt * 2.0f; //2 degrees/s
	waterCycle += dt * 0.25f; //10 units/s

	light->SetPosition(camera->GetPosition());

	sceneTime += dt;

	
	if (shadowLight->GetPosition().z > 100.0f) {
		lightDir = false;
	}

	if(shadowLight->GetPosition().z < 40.0f) {
		lightDir = true;
	}

	if (!lightDir) {
		shadowLight->SetPosition(Vector3(shadowLight->GetModifiablePosition().x, shadowLight->GetModifiablePosition().y, shadowLight->GetModifiablePosition().z - (dt * 5)));
		//lightDir = true;
	}

	if (lightDir) {
		shadowLight->SetPosition(Vector3(shadowLight->GetModifiablePosition().x, shadowLight->GetModifiablePosition().y, shadowLight->GetModifiablePosition().z + (dt * 5)));
	}
	

	if (deleteTimer > 2.5) {
		delete car;
		noCar = true;
		deleteTimer = 0.0f;
		rotatedCar = false;
	}

	if (noCar) {
		int street = rand() % 3;
		car = new SceneNode(cube);
		car->SetTransform(cubeRobot->GetTransform() * Matrix4::Translation(Vector3(-850 + (street * 500), -260, -700)));
		car->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		car->SetMesh(carMesh);
		car->SetTexture(earthTex);
		car->SetModelScale(Vector3(30, 30, 30));
		car->SetBoundingRadius(100000.0f);
		noCar = false;
	}

	if (car->GetTransform().GetPositionVector().z <= 2950) {
		car->SetTransform(car->GetTransform() * Matrix4::Translation(Vector3(0, 0, 2)));
	}

	if (car->GetTransform().GetPositionVector().z > 2950) {
		
		if (!rotatedCar) {
			deleteTimer += dt;
			if (deleteTimer < 1) {
				car->SetTransform(car->GetTransform() * Matrix4::Rotation(90.0f * dt, Vector3(0, 1, 0)));
				car->SetTransform(car->GetTransform() * Matrix4::Translation(Vector3(0, 0, 2)));
			}
			else {
				rotatedCar = true;
			}
		}
		else {
			car->SetTransform(car->GetTransform() * Matrix4::Translation(Vector3(0, 0, 2)));
			deleteTimer += dt;
		}
	}
	root->Update(dt);
}

void Renderer::DrawNode(SceneNode* n, Shader* currentShader) {
	if (n->GetMesh()) {
		if (n->GetLight() == nullptr) {
			SetShaderLight(*billBoardLight);
		}

		else {
			SetShaderLight(*n->GetLight());
		}

		if (n->GetAnimation() == nullptr) {
			if (n->GetMatTextures().empty()) {
				if (n->GetMesh() == heightMap) {
					SetShaderLight(*light);
					DrawHeightmap();
					DrawWater();
					if (!noCar) {
						DrawCar();
					}
					DrawTreePot();
					DrawCube();
				}
				
				else {
					Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
					glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"), 1, false, model.values);

					glUniform4fv(glGetUniformLocation(currentShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());

					nodeTexture = n->GetTexture();
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, nodeTexture);

					nodeBump = n->GetBump();
					glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "bumpTex"), 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, earthBump);


					glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "useTexture"), nodeTexture);

					n->Draw(*this);
				}
			}

			else {

				UpdateShaderMatrices();

				vector<Matrix4> frameMatrices;

				Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
				glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"), 1, false, model.values);

				glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

				const Matrix4* invBindPose = n->GetMesh()->GetInverseBindPose();
				const Matrix4* frameData = n->GetMesh()->GetBindPose();

				for (unsigned int i = 0; i < n->GetMesh()->GetJointCount(); ++i) {
					frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
				}

				int j = glGetUniformLocation(currentShader->GetProgram(), "joints");
				glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

				for (int i = 0; i < n->GetMesh()->GetSubMeshCount(); ++i) {
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, n->GetMatTextures().at(i));
					n->GetMesh()->DrawSubMesh(i);
				}
			}
		}
	
		else {
			BindShader(animShader);

			if (n->GetLight() == nullptr) {
				SetShaderLight(*billBoardLight);
			}

			else {
				SetShaderLight(*n->GetLight());
			}

			UpdateShaderMatrices();

			vector<Matrix4> frameMatrices;

			Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
			glUniformMatrix4fv(glGetUniformLocation(animShader->GetProgram(), "modelMatrix"), 1, false, model.values);

			glUniform4fv(glGetUniformLocation(animShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
			glUniform1i(glGetUniformLocation(animShader->GetProgram(), "diffuseTex"), 0);

			const Matrix4* invBindPose = n->GetMesh()->GetInverseBindPose();
			const Matrix4* frameData = n->GetAnimation()->GetJointData(n->GetCurrentFrame());

			for (unsigned int i = 0; i < n->GetMesh()->GetJointCount(); ++i) {
				frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
			}

			int j = glGetUniformLocation(animShader->GetProgram(), "joints");
			glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

			for (int i = 0; i < n->GetMesh()->GetSubMeshCount(); ++i) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, n->GetMatTextures().at(i));
				n->GetMesh()->DrawSubMesh(i);
			}
		}
	}
}

void Renderer::RenderScene() {
	BuildNodeLists(root);
	SortNodeLists();

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	DrawSkybox();
	DrawNodes();

	UpdateShaderMatrices();

	DrawShadowScene();
	DrawShadowedObjects();

	ClearNodeLists();
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawCar() {
	BindShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	Matrix4 model = car->GetTransform() * Matrix4::Scale(car->GetModelScale());
	glUniformMatrix4fv(glGetUniformLocation(lightShader->GetProgram(), "modelMatrix"), 1, false, model.values);

	glUniform4fv(glGetUniformLocation(lightShader->GetProgram(), "nodeColour"), 1, (float*)&car->GetColour());

	nodeTexture = car->GetTexture();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, nodeTexture);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "useTexture"), nodeTexture);

	car->Draw(*this);
}

void Renderer::DrawHeightmap() {
	BindShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	heightMap->Draw();
}

void Renderer::DrawTree() {
	BindShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	vector<Matrix4> frameMatrices;

	Matrix4 model = tree->GetWorldTransform() * Matrix4::Scale(tree->GetModelScale());
	glUniformMatrix4fv(glGetUniformLocation(lightShader->GetProgram(), "modelMatrix"), 1, false, model.values);

	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	const Matrix4* invBindPose = tree->GetMesh()->GetInverseBindPose();
	const Matrix4* frameData = tree->GetMesh()->GetBindPose();

	for (unsigned int i = 0; i < tree->GetMesh()->GetJointCount(); ++i) {
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(lightShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	for (int i = 0; i < tree->GetMesh()->GetSubMeshCount(); ++i) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tree->GetMatTextures().at(i));
		tree->GetMesh()->DrawSubMesh(i);
	}
}

void Renderer::DrawWater() {
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightmapSize();

	modelMatrix = Matrix4::Translation(hSize * 0.5f) * Matrix4::Scale(hSize * 0.5f) * Matrix4::Rotation(90, Vector3(1, 0, 0));

	UpdateShaderMatrices();
	SetShaderLight(*light);
	quad->Draw();
}

void Renderer::BuildNodeLists(SceneNode* from) {

	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}

		else {
			nodeList.push_back(from);
		}
	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists((*i));
	}
}

void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.rbegin(), transparentNodeList.rend(), SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::DrawNodes() {
	for (const auto& i : nodeList) {
		BindShader(lightShader);
		DrawNode(i, lightShader);
	}
	for (const auto& i : transparentNodeList) {
		BindShader(basicSceneShader);
		DrawNode(i, basicSceneShader);
	}
}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}

void Renderer::DrawShadowScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	for (int i = 0; i < sceneMeshes.size(); ++i) {
		
		BindShader(shadowShader);
		SetShaderLight(*shadowLight);
		viewMatrix = Matrix4::BuildViewMatrix(shadowLight->GetPosition(), Vector3(100, 230, 100));
		projMatrix = Matrix4::Perspective(1, 145, 1, 45);
		shadowMatrix = projMatrix * viewMatrix;
		
		modelMatrix = sceneTransforms[i];
		UpdateShaderMatrices();
		sceneMeshes[i]->Draw();
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawShadowedObjects() {
	BindShader(sceneShader);
	SetShaderLight(*shadowLight);
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex"), 2);

	glUniform3fv(glGetUniformLocation(sceneShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneDiffuse);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, sceneBump);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	for (int i = 0; i < sceneMeshes.size(); ++i) {
		modelMatrix = sceneTransforms[i];
		UpdateShaderMatrices();
		sceneMeshes[i]->Draw();
	}
}

void Renderer::DrawCube() {
	BindShader(blendShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(blendShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1f(glGetUniformLocation(blendShader->GetProgram(), "time"), sceneTime);

	glUniform1f(glGetUniformLocation(blendShader->GetProgram(), "yCoord"), blendCube->GetWorldTransform().GetPositionVector().y);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	Matrix4 model = blendCube->GetTransform() * Matrix4::Scale(blendCube->GetModelScale());
	glUniformMatrix4fv(glGetUniformLocation(blendShader->GetProgram(), "modelMatrix"), 1, false, model.values);

	glUniform4fv(glGetUniformLocation(blendShader->GetProgram(), "nodeColour"), 1, (float*)&blendCube->GetColour());

	nodeTexture = blendCube->GetTexture();
	glUniform1i(glGetUniformLocation(blendShader->GetProgram(), "mainTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, nodeTexture);

	nodeTexture = blendCube->GetSecondTexture();
	glUniform1i(glGetUniformLocation(blendShader->GetProgram(), "secondTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, nodeTexture);

	glUniform1i(glGetUniformLocation(blendShader->GetProgram(), "useTexture"), nodeTexture);

	blendCube->Draw(*this);
}

void Renderer::DrawTreePot() {
	BindShader(combineShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(combineShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1f(glGetUniformLocation(combineShader->GetProgram(), "time"), sceneTime);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	Matrix4 model = treePot->GetTransform() * Matrix4::Scale(treePot->GetModelScale());
	glUniformMatrix4fv(glGetUniformLocation(combineShader->GetProgram(), "modelMatrix"), 1, false, model.values);

	glUniform4fv(glGetUniformLocation(combineShader->GetProgram(), "nodeColour"), 1, (float*)&treePot->GetColour());

	nodeTexture = treePot->GetTexture();
	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "mainTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, nodeTexture);

	nodeTexture = treePot->GetSecondTexture();
	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "secondTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, nodeTexture);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "useTexture"), nodeTexture);

	treePot->Draw(*this);
}
