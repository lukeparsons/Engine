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

template<size_t row, size_t column>
class Grid2D
{
private:
	const float cellWidth;
public:

	float density;
	float temperature;
	float concentration;

	std::map<CellLocation, Cell2D> centerCells;
	std::map<CellLocation, BorderCell2D> borderCells;

	std::map<CellLocation, float> uField;
	std::map<CellLocation, float> vField;
	std::map<CellLocation, float> pressure;

	std::map<CellLocation, float> ADiag;
	std::map<CellLocation, float> Ax;
	std::map<CellLocation, float> Ay;

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

				pressure.insert({ {i, j}, 0 });

				uField.insert({ {i, j}, 0 }); // right arrow of cell
				vField.insert({ {i, j}, 0 }); // top arrow of cell

				if(i == 0)
				{
					BorderCell2D borderCell(render, BorderCell2D::LEFT);
					uField.insert({ {i - 1, j}, 0 }); // left arrow of left-most cells

					if(j == 0)
					{
						vField.insert({ {i, j - 1}, 0 });
						borderCell.borderType = BorderCell2D::TOP_LEFT;
					} else if(j == column - 1)
					{
						borderCell.borderType = BorderCell2D::BOTTOM_LEFT;
					}
					borderCells.insert({ cellGridLoc, borderCell });
					continue;
				} else if(i == row - 1)
				{
					BorderCell2D borderCell(render, BorderCell2D::RIGHT);

					if(j == 0)
					{
						borderCell.borderType = BorderCell2D::TOP_RIGHT;
					} else if(j == column - 1)
					{
						borderCell.borderType = BorderCell2D::BOTTOM_RIGHT;
					}
					borderCells.insert({ cellGridLoc, borderCell });
					continue;
				}

				if(j == 0)
				{
					vField.insert({ {i, j - 1}, 0 }); // bottom arrows of bottom-most cells
					borderCells.insert({ cellGridLoc, BorderCell2D(render, BorderCell2D::TOP) });
					continue;
				} else if(j == column - 1)
				{
					borderCells.insert({ cellGridLoc, BorderCell2D(render, BorderCell2D::BOTTOM) });
					continue;
				}

				centerCells.insert({ cellGridLoc, Cell2D(render) });
			}
		}
	}

	void Update(float timeStep)
	{
		float scale = 1 / (density * cellWidth);

		for(auto& [location, cell] : centerCells)
		{
			cell.cellState = Cell2D::FLUID;
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

			float negativeDivergence = -(uField[location.i + 1, location.j] - uField[location.i - 1, location.j] +
				vField[location.i + 1, location.j] - vField[location.i - 1, location.j]) / cellWidth;

			if(cell.cellState == Cell2D::FLUID)
			{
				if()
			}
		}

		for(auto& [location, cell] : borderCells)
		{
			cell.renderComponent->ChangeTexture(solidTexture);
		}
	}

};