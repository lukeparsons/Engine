#pragma once
#include "../../../scene/RenderedObject.h"

class EulerianGrid : public RenderedObject
{
private:
	size_t row, column;
public:
	EulerianGrid(const size_t row, const size_t column, const Mesh& model, const Vector3f& location);
};

