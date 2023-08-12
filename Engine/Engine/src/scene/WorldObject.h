#pragma once
#include "../math/Vectorf.h"

class WorldObject
{
public:

	WorldObject(const Vector3f& location = Vector3f(0, 0, 0), const Vector3f& scale = Vector3f(1, 1, 1)) :
		location(location), scale(scale) {};

	Vector3f location;
	Vector3f rotation;
	Vector3f scale;
};