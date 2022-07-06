#pragma once
#include "Matrixf.h"

struct VectorMatrix3f : public Matrixf<4, 1>
{

	VectorMatrix3f(float x, float y, float z);

	float& x();
	float& y();
	float& z();
};
