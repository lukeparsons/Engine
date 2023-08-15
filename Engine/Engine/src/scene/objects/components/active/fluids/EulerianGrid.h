#pragma once
#include "../ActiveComponent.h"
#include "../../../WorldObject.h"
#include "../RenderComponent.h"

class Cell
{
public:
	WorldObject* object;
	Cell(const Mesh& model, TransformComponent transform = TransformComponent())
	{
		object = CreateModel(model, transform);
	};
};

class EulerianGrid : public ActiveComponent
{
private:
	size_t row, column;
public:
	std::vector<Cell*> cells;

	EulerianGrid(const size_t row, const size_t column, const Mesh& gridModel, const Vector3f& location);

	void FrameUpdate();
};

