#pragma once

/*class Cell
{
public:
	WorldObject object;
	Cell(const Mesh& model, const TransformComponent& transform)
	{
		object = CreateModel(model, transform);
	};
};

class EulerianGrid : public ActiveComponent
{
private:
	size_t row, column;
	virtual ActiveComponent* cloneComponent() const
	{
		return new EulerianGrid(*this);
	}

public:
	std::vector<Cell*> cells;

	EulerianGrid(const size_t row, const size_t column, const Mesh& gridModel, const Vector3f& location);

	EulerianGrid(const EulerianGrid& other, WorldObject& newWorldObject) : row(other.row), column(other.column), cells(other.cells) {};

	void FrameUpdate();
}; */

