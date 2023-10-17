#pragma once
#include "GridCell2D.h"
#include <map>

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

	Value& operator[](const Key& key)
	{
		typename std::map<Key, Value>::iterator it = map.find(key);
		return it != map.end() ? it->second : defaultValue;
	}

	Value operator[](const Key& key) const
	{
		return this[key];
	}

	void insert(const Key& key, const Value& value)
	{
		map[key] = value;
	}

	void insert(const Key& key, const Value&& value)
	{
		map.emplace(key, std::move(value));
	}

	std::map<Key, Value>::iterator begin()
	{ 
		return map.begin();
	}

	std::map<Key, Value>::iterator end()
	{
		return map.end();
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

	GridDataMap<CellLocation, Cell2D> cells = GridDataMap<CellLocation, Cell2D>(Cell2D());

	GridDataMap<CellLocation, float> uField = GridDataMap<CellLocation, float>(0);
	GridDataMap<CellLocation, float> vField = GridDataMap<CellLocation, float>(0);
	GridDataMap<CellLocation, float> pressure = GridDataMap<CellLocation, float>(0);

	/* These 'A' named variables store the coefficient matrix for the pressure calculations
	Each row of the matrix corresponds to one fluid cell
	The entries in that row are the coefficients of all the pressure unknowns in the equation for that cell.
	These are the pressure values of the cell's neighbours
	A is symmetric: For example the coefficient of p(i + 1, j, k) in the equation for cell (i, j, k) is stored at
	A(i, j, k),(i + 1, j, k) and must be equal to A(i + 1, j, k)(i, j, k) 
	We store three numbers at every grid cell, one for the diagonal entry (i.e, the cell itself), one for the cell to the right and one for the cell directly up 
	When we need to refer to an entry like A(i, j)(i - 1, j) we use the symmetry property and instead use A(i - 1, j)(i ,j) = Ax(i - 1, j)
	Thus we only need to store the coefficient for the positive direction in each row */

	GridDataMap<CellLocation, float> ADiag = GridDataMap<CellLocation, float>(0); // ADiag stores the coefficient for A(i, j)(i, j)
	GridDataMap<CellLocation, float> Ax = GridDataMap<CellLocation, float>(0); // Ax stores the coefficient for A(i, j)(i + 1, j)
	GridDataMap<CellLocation, float> Ay = GridDataMap<CellLocation, float>(0); // Ax stores the coefficient for A(i, j)(i, j + 1)

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

				cells.insert(cellGridLoc, Cell2D(Cell2D::SOLID, render));
			}
		}
	}

	double StandardPCG()
	{
		// PCG algorithm for solving Ap = b
		double pressureGuess = 0;
		Vector2f residualVector = b; // TODO b is negative divergences

		Vector2f auxiliaryVector = applyPreconditoner(r);
		Vector2f searchVector = z;

		double sigma = dotproduct(z, r);

		for(unsigned int i = 0; i < 200; i++) // 200 here is max iterations
		{
			auxiliaryVector = applyA(searchVector);
			Vector2f alpha = sigma / dotproduct(z, s);
			pressureGuess = pressureGuess + alpha * sigma;
			residualVector = residualVector - alpha * auxiliaryVector;

			if(max(residualVector) <= tolerance)
			{
				return pressureGuess;
			}

			auxiliaryVector = applyPreconditoner(r);
			double sigmaNew = dotproduct(auxiliaryVector, residualVector);
			double beta = sigmaNew / sigma;

			searchVector = auxiliaryVector + beta * searchVector;

			sigma = sigmaNew;
		}

		// Iteration limit exceeded
		return pressureGuess;
	}

	void Update(float timeStep)
	{
		float scale = 1 / (density * cellWidth);
		float Acoefficient = timeStep / (density * cellWidth * cellWidth);
		
		uField[location] -= timeStep * scale * pressure[{location.i + 1, location.j}] - pressure[location];
		vField[location] -= timeStep * scale * pressure[{location.i, location.j + 1}] - pressure[location];

		float negativeDivergence = -(uField[{location.i + 1, location.j}] - uField[{location.i - 1, location.j}] +
			vField[{location.i + 1, location.j}] - vField[{location.i - 1, location.j}]) / cellWidth;

		for(auto& [location, cell] : cells)
		{
			switch(cell.cellState)
			{
				case Cell2D::SOLID:
					cell.renderComponent->ChangeTexture(solidTexture);
					break;
				case Cell2D::FLUID:
					cell.renderComponent->ChangeTexture(fluidTexture);

					// Pressure coefficient update

					float* cellPressureCoefficient = &ADiag[{ location.i, location.j }];

					if(cells[{ location.i - 1, location.j }].cellState == Cell2D::FLUID) // Left neighbour
					{
						*cellPressureCoefficient += scale;
					}

					Cell2D::State positiveXCellState = cells[{ location.i + 1, location.j }].cellState;
					switch(positiveXCellState)
					{
						case Cell2D::FLUID:
							*cellPressureCoefficient += scale;
							Ax[{ location.i, location.j }] = -scale;
							break;
						case Cell2D::EMPTY:
							*cellPressureCoefficient += scale;
							break;
						default:
							break;
					}

					if(cells[{ location.i, location.j - 1 }].cellState == Cell2D::FLUID) // Below neighbour
					{
						*cellPressureCoefficient += scale;
					}

					Cell2D::State positiveYCellState = cells[{ location.i, location.j + 1 }].cellState;
					switch(positiveXCellState)
					{
						case Cell2D::FLUID:
							*cellPressureCoefficient += scale;
							Ax[{ location.i, location.j + 1 }] = -scale;
							break;
						case Cell2D::EMPTY:
							*cellPressureCoefficient += scale;
							break;
						default:
							break;
					}

					break;
				case Cell2D::EMPTY:
					cell.renderComponent->ChangeTexture(emptyTexture);
					break;
			}

			
			StandardPCG();
		}
	}

};