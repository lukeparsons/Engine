#include "EulerianGrid.h"

/*EulerianGrid::EulerianGrid(const size_t row, const size_t column, const Mesh& gridModel, const Vector3f& location)
	: row(row), column(column)
{
	for(int i = 0; i < row; i++)
	{
		for(int j = 0; j < column; j++)
		{
			cells.emplace_back(new Cell(gridModel, 
				TransformComponent(location + Vector3f((float)(i * 2), 0, (float)(j * 2)), Vector3f(0.1f, 0.1f, 0.1f))));
		}
	}
}

void EulerianGrid::FrameUpdate()
{
	for(Cell* c : cells)
	{
		c->object.FrameUpdateComponents();
	}
} */