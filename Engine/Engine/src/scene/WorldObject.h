#pragma once
#include "../math/Matrix4f.h"

class WorldObject
{
private:
	float* vertices;
public:
	VectorMatrix3f location;
	VectorMatrix3f rotation;
	VectorMatrix3f scale;
};

