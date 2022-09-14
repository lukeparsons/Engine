#pragma once
#include "WorldObject.h"
#include "../math/Matrix4f.h"

class Camera : public WorldObject {

public:

	Camera(Vector3f location);

	const float moveSpeed = 0.001f;

	Vector3f target;
	Vector3f look;

	Matrix4f GetCameraSpaceMatrix();

};