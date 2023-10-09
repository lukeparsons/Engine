#pragma once
#include "GridCell2D.h"

static const std::shared_ptr<Texture> fluidTexture = std::make_shared<Texture>("../Engine/assets/smoke.png");
static const std::shared_ptr<Texture> solidTexture = std::make_shared<Texture>("../Engine/assets/block.png");
static const std::shared_ptr<Texture> emptyTexture = std::make_shared<Texture>("../Engine/assets/wall2.png");

struct CellLocation
{
	unsigned int i, j;

	CellLocation(unsigned int _i, unsigned int _j) : i(_i), j(_j) {};

	bool operator<(const CellLocation& rhs) const
	{
		return i == rhs.i ? j < rhs.j : i < rhs.i;
	}
};

template<typename Key, typename Value>
class GridDataMap
{
private:
	std::map<Key, Value> map;
	Value defaultValue;
public:
	
	GridDataMap(const Value& _defaultValue) : defaultValue(_defaultValue) {};

	Value operator[](const Key& key) const
	{
		return map.contains(key) ? map[key] : defaultValue;
	}

	Value& operator[](const Key& key)
	{
		return map.contains(key) ? map[key] : defaultValue;
	}

	void insert(const Key& key, const Value& value)
	{
		map[key] = value;
	}

};

template<size_t row, size_t column>
class Grid2D
{
private:
	const float cellWidth;
public:

	float density;
	float temperature;
	float concentration;

	std::map<CellLocation, Cell2D> cells;

	GridDataMap<CellLocation, float> uField = GridDataMap<CellLocation, float>(0);
	GridDataMap<CellLocation, float> vField = GridDataMap<CellLocation, float>(0);
	GridDataMap<CellLocation, float> pressure = GridDataMap<CellLocation, float>(0);

	GridDataMap<CellLocation, float> ADiag = GridDataMap<CellLocation, float>(0);
	GridDataMap<CellLocation, float> Ax = GridDataMap<CellLocation, float>(0);
	GridDataMap<CellLocation, float> Ay = GridDataMap<CellLocation, float>(0);

	Grid2D(Scene& scene, std::shared_ptr<Mesh>& gridModel, const Vector2f& location, float _density, float _cellWidth) : density(_density), cellWidth(_cellWidth)
	{
		Vector3f cellScale = Vector3f(cellWidth, cellWidth, cellWidth);

		for(unsigned int i = 0; i < row; i++)
		{
			for(unsigned int j = 0; j < column; j++)
			{
				EntityID cellID = scene.CreateModel(gridModel, solidTexture,
					Vector3f(location.x, location.y, 0) + Vector3f(i * cellWidth * 2, j * cellWidth * 2, 0), cellScale);
				RenderComponent* render = scene.GetComponent<RenderComponent>(cellID);

				CellLocation cellGridLoc = { i, j };

				pressure.insert({i, j}, 0);

				uField.insert({i, j}, 0); // right arrow of cell
				vField.insert({i, j}, 0); // top arrow of cell

				if(i == 0)
				{
					uField.insert({i - 1, j}, 0); // left arrow of left-most cells
				}

				if(j == 0)
				{
					vField.insert({i, j - 1}, 0); // bottom arrows of bottom-most cells
				}

				cells.insert({ cellGridLoc, Cell2D(render) });
			}
		}
	}

	void Update(float timeStep)
	{
		float scale = 1 / (density * cellWidth);

		for(auto& [location, cell] : cells)
		{
			switch(cell.cellState)
			{
				case Cell2D::SOLID:
					cell.renderComponent->ChangeTexture(solidTexture);
					break;
				case Cell2D::FLUID:
					cell.renderComponent->ChangeTexture(fluidTexture);
					break;
				case Cell2D::EMPTY:
					cell.renderComponent->ChangeTexture(emptyTexture);
					break;
			} 

			uField[location] -= timeStep * scale * pressure[{location.i + 1, location.j}] - pressure[location];
			vField[location] -= timeStep * scale * pressure[{location.i, location.j + 1}] - pressure[location];

			float negativeDivergence = -(uField[{location.i + 1, location.j}] - uField[{location.i - 1, location.j}] +
				vField[{location.i + 1, location.j}] - vField[{location.i - 1, location.j}]) / cellWidth;
		}
	}

};