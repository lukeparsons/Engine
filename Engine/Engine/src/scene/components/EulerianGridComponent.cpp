#include "EulerianGridComponent.h"
#include "RenderComponent.h"
#include "../Scene.h"

void EulerianGridComponent::InitializeGrid(Scene& scene, Mesh& gridModel, size_t _row, size_t _column, size_t _depth, const Vector3f& location)
{
	row = _row;
	column = _column;
	depth = _depth;
	for(int i = 0; i < row; i++)
	{
		for(int j = 0; j < column; j++)
		{
			for(int k = 0; k < depth; k++)
			{
				//EntityID cellID = scene.CreateModel(gridModel, location + Vector3f((float)(i * 2), (float)(j * 2), (float)(k * 2)), Vector3f(1.f, 0.1f, 0.1f));
				//cells.push_back(Cell(cellID, std::make_shared<Mesh>(scene.GetComponent<RenderComponent>(cellID)->mesh)));
			}

		}
	}
}
