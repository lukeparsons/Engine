#include "EulerianGridComponent.h"

void EulerianGridComponent::InitializeGrid(Scene& scene, const size_t _row, const size_t _column, const Mesh& gridModel, const Vector3f& location)
{
	row = _row;
	column = _column;
	for(int i = 0; i < row; i++)
	{
		for(int j = 0; j < column; j++)
		{
			EntityID cellID = scene.CreateModel(gridModel, location + Vector3f((float)(i * 2), (float)(j * 2), 0), Vector3f(0.1f, 0.1f, 0.1f));
			cells.push_back(Cell(cellID));
		}
	}
}