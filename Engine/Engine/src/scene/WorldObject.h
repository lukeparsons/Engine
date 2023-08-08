#pragma once
#include "../math/Vectorf.h"

class WorldObject
{
public:

	WorldObject(Vector3f location = Vector3f(0, 0, 0));

	Vector3f location;
	Vector3f rotation;
	Vector3f scale;
};