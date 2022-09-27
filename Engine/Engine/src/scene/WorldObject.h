#pragma once
#include "../math/Vector3f.h"

class WorldObject
{
private:

	float* vertices;

protected:

	WorldObject(Vector3f location);

public:

	WorldObject(Vector3f location, float vertices[]);

	Vector3f location;
	Vector3f rotation;
	Vector3f scale;
};

