#include "SceneNode.h"
#include "MeshAnimation.h"
#include "MeshMaterial.h"

SceneNode::SceneNode(Mesh* mesh, Vector4 colour) {
	this->mesh = mesh;
	this->colour = colour;
	parent = NULL;
	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;
	texture = 0;
	secondTexture = 0;
	modelScale = Vector3(1, 1, 1);
	frameTime = 0.0f;
	currentFrame = 0;
	animation = nullptr;
	light = nullptr;
}

SceneNode::~SceneNode(void) {
	for (unsigned int i = 0; i < children.size(); ++i) {
		delete children[i];
	}
}

void SceneNode::AddChild(SceneNode* s) {
	children.push_back(s);
	s->parent = this;
}

//this function can disable depth testing, does more advanced rendering
void SceneNode::Draw(const OGLRenderer& r) {
	if (mesh) { mesh->Draw(); }
}

void SceneNode::SetMatTextures() {

	vector<GLuint> matTextures;

	for (int i = 0; i < this->GetMesh()->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = this->GetMaterial()->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		if (filename) {
			string path = TEXTUREDIR + *filename;
			GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
			matTextures.emplace_back(texID);
		}
		else {
			matTextures.emplace_back(0); //No texture :(
		}
	}

	this->matTextures = matTextures;
}

void SceneNode::Update(float dt) {
	if (parent) {
		worldTransform = parent->worldTransform * transform;
	}
	else {
		worldTransform = transform;
	}

	if (!matTextures.empty() && animation != nullptr) {
		frameTime -= dt;

		while (frameTime < 0.0f) {
			currentFrame = (currentFrame + 1) % animation->GetFrameCount();
			frameTime += 1.0f / animation->GetFrameRate();
		}
	}

	for (vector<SceneNode*>::iterator i = children.begin(); i != children.end(); ++i) {
		(*i)->Update(dt);
	}
}