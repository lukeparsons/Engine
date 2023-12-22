#pragma once
#include "Grid2D.h"

template<size_t row, size_t column>
class Smoke : public Grid2D
{
	Smoke(unsigned int _row, unsigned int _column, Scene& scene, std::shared_ptr<Mesh>& gridModel, const Vector2f& location, float _density, float _cellWidth)
		: Grid2D(_row, _column, scene, gridModel, location, _density, _cellWidth) {};
};