#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include <vector>

class Camera {
public:
	Camera(void) {
		yaw = 0.0f;
		pitch = 0.0f;
	}

	Camera(float pitch, float yaw, Vector3 position) {
		this->pitch = pitch;
		this->yaw = yaw;
		this->position = position;
	}

	~Camera(void) {};

	void UpdateCamera(float dt = 1.0f);

	Matrix4 BuildViewMatrix();

	Vector3 GetPosition() const { return position; }
	void SetPosition(Vector3 val) { position = val; }

	float GetYaw() const { return yaw; }
	void SetYaw(float y) { yaw = y; }

	float GetPitch() const { return pitch; }
	void SetPitch(float p) { pitch = p; }

protected:
	
	float yaw;
	float pitch;
	Vector3 position;
	Vector3 railDirection;
	Vector3 railDestination;
	std::vector<Vector3> railDests = { Vector3(4120, 1275, 0), Vector3(4120, 1275, 4120), Vector3(2060, 2075, 2060), Vector3(0, 1275, 0) };
	std::vector<Vector3> railDirs = { Vector3(4125, 0, 0) /  1000, Vector3(0, 0, 4120) / 1000, Vector3(-2060, 800, -2060) / 1000, Vector3(-2060, -800, -2060) / 1000 };
	//vector<Vector3> railDests;
	bool moving;
	bool yawShift = false;
	double e = 20;
	int count = 0;
	int speedFactor = 2;
};