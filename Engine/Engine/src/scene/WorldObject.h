#pragma once
#include "../math/Matrix4f.h"

class WorldObject
{
private:

	float* vertices;

public:

	Vector3f location;
	Vector3f rotation;
	Vector3f scale;
};

