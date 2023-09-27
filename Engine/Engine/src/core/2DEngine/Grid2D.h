#pragma once
#include "GridCell2D.h"

static const std::shared_ptr<Texture> fluidTexture = std::make_shared<Texture>("../Engine/assets/smoke.png");
static const std::shared_ptr<Texture> solidTexture = std::make_shared<Texture>("../Engine/assets/block.png");
static const std::shared_ptr<Texture> emptyTexture = std::make_shared<Texture>("../Engine/assets/wall2.png");

template<size_t row, size_t column>
class Grid2D
{
private:
	const float cellWidth;
public:

	float density;
	float temperature;
	float concentration;

	std::map<IntPair, Cell2D> centerCells;
	std::map<IntPair, BorderCell2D> borderCells;

	std::map<IntPair, float> uField;
	std::map<IntPair, float> vField;
	std::map<IntPair, float> pressure;

	Matrix<float, row, column> divergCoeff;
	Matrix<float, row, 1> negativeDiverg;

	Grid2D(Scene& scene, std::shared_ptr<Mesh>& gridModel, const Vector2f& location, float _density, float _cellWidth) : density(_density), cellWidth(_cellWidth)
	{
		Vector3f cellScale = Vector3f(cellWidth, cellWidth, cellWidth);

		for(int i = 0; i < row; i++)
		{
			for(int j = 0; j < column; j++)
			{
				EntityID cellID = scene.CreateModel(gridModel, solidTexture,
					Vector3f(location.x, location.y, 0) + Vector3f(i * cellWidth * 2, j * cellWidth * 2, 0), cellScale);
				RenderComponent* render = scene.GetComponent<RenderComponent>(cellID);

				IntPair cellGridLoc = { i, j };

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

			uField[location] -= timeStep * scale * pressure[{location.first + 1, location.second}] - pressure[location];
			vField[location] -= timeStep * scale * pressure[{location.first, location.second + 1}] - pressure[location];
		}

		for(auto& [location, cell] : borderCells)
		{
			cell.renderComponent->ChangeTexture(solidTexture);
		}
	}

};