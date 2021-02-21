#include "Camera.h"
#include "Window.h"
#include <algorithm>

void Camera::UpdateCamera(float dt) {
	if (!yawShift) {
		pitch -= (Window::GetMouse()->GetRelativePosition().y);
		yaw -= (Window::GetMouse()->GetRelativePosition().x);
	}
	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);

	if (yaw < 0) {
		yaw += 360.0f;
	}

	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));

	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);

	float speed = 30.0f * (dt * speedFactor);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_6)) {
		if (!moving) {
			position = Vector3(0, 1275, 0);
			moving = true;
		}
		else{
			moving = false;
		}
	}

	if (moving) {

		railDestination = railDests.at(count);

		//railDirection = (railDests.at(count) - position) / 1000;

		if (count == 0) {
			if (!yawShift) {
				SetPitch(-17.0f);
				SetYaw(220.0f);
				yawShift = true;
			}
			yaw -= 80.0f / 1000;
		}

		if (count == 1) {
			yaw -= 80.0f / 1000;
		}

		if (count == 2) {

			yaw -= 40.0f / 1000.0f;
			pitch -= 73.0f / 1000.0f;
			
		}

		if (count == 3) {
			yaw += 200.0f / 1000.0f;
			pitch += 73.0f / 1000.0f;
		}

		position += railDirs[count];

		if ((abs(position.x - railDestination.x) < e) && (abs(position.y - railDestination.y) < e) && abs(position.z - railDestination.z) < e) {
			if (count == 3) {
				yawShift = false;
			}
			count++;
		}

		if (count == railDests.size()) {
			//moving = false;
			count = 0;
		}
	}

	if (!moving) {
		yawShift = false;
		count = 0;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_PLUS)) {
		speedFactor++;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_MINUS) && speedFactor > 1) {
		speedFactor--;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W) && !moving) {
		position += forward * speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S) && !moving) {
		position -= forward * speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A) && !moving) {
		position -= right * speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D) && !moving) {
		position += right * speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT) && !moving) {
		position.y += speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE) && !moving) {
		position.y -= speed;
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_5) && !moving) {
		position = Vector3(10, 10, 10);
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
		position = Vector3(2040, 220, 2040);

		railDestination = Vector3(2500, 1475, 2000);

		railDirection = (railDestination - position) / 600;

		moving = true;
	}
}

Matrix4 Camera::BuildViewMatrix() {
	return Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
		   Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
		   Matrix4::Translation(-position);
}
