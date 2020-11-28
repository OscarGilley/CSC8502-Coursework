#pragma once

#include "Plane.h"
class SceneNode;
class Matrix4; //lets complier know that these keywords are classes, without include the whole header

class Frustum {
public:
	Frustum(void) {};
	~Frustum(void) {};

	void FromMatrix(const Matrix4& mvp);
	bool InsideFrustum(SceneNode& n);

protected:
	Plane planes[6];
};