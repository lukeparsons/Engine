#include "EulerianGrid.h"

EulerianGrid::EulerianGrid(const size_t row, const size_t column, const Mesh& gridModel, const Vector3f& location)
	: row(row), column(column), RenderedObject(gridModel, location)
{

}
