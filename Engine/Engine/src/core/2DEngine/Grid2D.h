#pragma once
#include "GridCell2D.h"
#include "GridStructures.h"
#include <map>

static const std::shared_ptr<Texture> fluidTexture = std::make_shared<Texture>("../Engine/assets/smoke.png");
static const std::shared_ptr<Texture> solidTexture = std::make_shared<Texture>("../Engine/assets/block.png");
static const std::shared_ptr<Texture> emptyTexture = std::make_shared<Texture>("../Engine/assets/wall2.png");

template<size_t row, size_t column>
class Grid2D
{
	using GridRowVector = RowVector<double, row * column>;
private:
	const float cellWidth;

	GridRowVector precon = GridRowVector(0);
	GridRowVector q = GridRowVector(0);
	GridRowVector z = GridRowVector(0);
	GridRowVector negativeDivergences = GridRowVector(0);
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

		GridRowVector residualVector = negativeDivergences; // TODO: Try changing these rowvector types to doubles 

		GridRowVector auxiliaryVector = applyPreconditioner(residualVector);
		GridRowVector searchVector = auxiliaryVector;

		double sigma = DotProduct(auxiliaryVector, residualVector);

		// TODO: Implement proper scaling
		double tolerance = 0.00001;

		for(unsigned int i = 0; i < 200; i++) // 200 here is max iterations
		{
			auxiliaryVector = applyA(searchVector);
			double alpha = sigma / DotProduct(auxiliaryVector, searchVector);
			pressureGuess = pressureGuess + alpha * sigma;
			residualVector = residualVector - (alpha * auxiliaryVector);

			if(max(residualVector) <= tolerance)
			{
				return pressureGuess;
			}

			auxiliaryVector = applyPreconditioner(residualVector);
			double sigmaNew = DotProduct(auxiliaryVector, residualVector);
			double beta = sigmaNew / sigma;

			searchVector = auxiliaryVector + (beta * searchVector);

			sigma = sigmaNew;
		}

		// Iteration limit exceeded
		return pressureGuess;
	}

	GridRowVector applyA(const GridRowVector& vector)
	{
		return vector;
	}

	/* Modified Incomplete Choleksy preconditoner to find a matrix M where M is approx A^ {-1}
	 Simple to implement, fairly efficient and robust in handling irregular domains (e.g liquid splash)
	 However hard to efficiently parallelize and not optimally scalable
	 */
	GridRowVector applyPreconditioner(GridRowVector& residualVector)
	{
		double tuningConstant = 0.97;
		double safetyConstant = 0.25;

		double e = 0;
		double cellPressureCoefficient = 0;
		double t = 0;
		for(auto& [location, cell] : cells)
		{
			if(cell.cellState == Cell2D::FLUID)
			{
				double Aplusi = Ax[{location.i - 1, location.j}];
				double Aplusj = Ay[{location.i, location.j - 1}];

				// NOTE: These will be the same as the precon from the last iteration of applyPreconditoner

				double previousXPrecon = precon[location.i - 1 + location.j];
				double previousYPrecon = precon[location.i + location.j - 1];

				cellPressureCoefficient = ADiag[{location.i, location.j}];

				e = cellPressureCoefficient - pow((Aplusi * previousXPrecon), 2)
					- pow((Aplusj - 1) * previousYPrecon, 2);

				e -= tuningConstant * (Aplusi * (Aplusj * pow(previousXPrecon, 2))
					+ Aplusj * (Aplusi * previousYPrecon, 2));

				if(e < safetyConstant * cellPressureCoefficient)
				{
					e = cellPressureCoefficient;
				}

				precon[location.i + location.j] = 1 / sqrt(e);

				// Solve Lq = r
				t = residualVector[location.i + location.j]
					- Aplusi * previousXPrecon * q[location.i - 1 + location.j]
					- Aplusj * previousYPrecon * q[location.i + location.j - 1];

				q[location.i, location.j] = t * precon[location.i + location.j];

			}
		}
		for(std::map<CellLocation, Cell2D>::reverse_iterator rit = cells.rbegin(); rit != cells.rend(); ++rit)
		{
			CellLocation location = rit->first;
			if(rit->second.cellState == Cell2D::FLUID)
			{
				t = q[location.i, location.j]
					- Ax[{location.i, location.j}] * precon[location.i + location.j] * z[location.i + 1, location.j]
					- Ay[{location.i, location.j}] * precon[location.i + location.j] * z[location.i + 1, location.j];

				z[location.i + location.j] = t * precon[location.i, location.j];

			}
		}

		return precon;
	}

	void Update(float timeStep)
	{
		float scale = 1 / (density * cellWidth);
		float Acoefficient = timeStep / (density * cellWidth * cellWidth);
	
		for(auto& [location, cell] : cells)
		{
			uField[location] -= timeStep * scale * pressure[{location.i + 1, location.j}] - pressure[location];
			vField[location] -= timeStep * scale * pressure[{location.i, location.j + 1}] - pressure[location];
			negativeDivergences[location.i + location.j] = -(uField[{location.i + 1, location.j}] - uField[{location.i - 1, location.j}] +
				vField[{location.i + 1, location.j}] - vField[{location.i - 1, location.j}]) / cellWidth;

			switch(cell.cellState)
			{
				case Cell2D::SOLID:
					//cell.renderComponent->ChangeTexture(solidTexture);
					break;
				case Cell2D::FLUID:
				{
					//cell.renderComponent->ChangeTexture(fluidTexture);

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
				}
					break;
				case Cell2D::EMPTY:
					//cell.renderComponent->ChangeTexture(emptyTexture);
					break;
			}

			
			//StandardPCG();
		}
	}

};