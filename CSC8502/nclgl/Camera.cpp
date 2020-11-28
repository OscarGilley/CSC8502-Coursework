#include "Camera.h"
#include "Window.h"
#include <algorithm>

void Camera::UpdateCamera(float dt) {
	pitch -= (Window::GetMouse()->GetRelativePosition().y);
	yaw -= (Window::GetMouse()->GetRelativePosition().x);

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

	if (moving) {
		position += railDirection;

		if ((abs(position.x - railDestination.x) < e) && (abs(position.y - railDestination.y) < e) && abs(position.z - railDestination.z) < e) {
			moving = false;
		}
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

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_6) && !moving) {
		position = Vector3(1500, 1275, 1500);

		railDestination = Vector3(2500, 1475, 2000);

		railDirection = (railDestination - position) / 600;

		moving = true;
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
