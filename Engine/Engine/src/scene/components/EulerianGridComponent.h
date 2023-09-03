#pragma once
#include "../../types/EngineTypes.h"
#include "../Scene.h"

class Cell
{
public:
	EntityID id;
	Cell(EntityID _id) : id(_id) {};
};

class EulerianGridComponent : public Component
{
private:
	size_t row, column;

public:
	std::vector<Cell> cells;

	EulerianGridComponent(EntityID id, std::unordered_map<std::type_index, Component*>& entityComponents) : Component(id), row(0), column(0) {};
	void InitializeGrid(Scene& scene, const size_t row, const size_t column, const Mesh& gridModel, const Vector3f& location);
};

