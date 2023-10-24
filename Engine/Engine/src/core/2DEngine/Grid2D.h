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
private:
	const float cellWidth;

public:

	float density;
	float temperature;
	float concentration;

	GridStructure<GridDataPoint, row, column> gridData = GridStructure<GridDataPoint, row, column>(GridDataPoint(Cell2D(nullptr), GridDataPoint::EMPTY));
	RowVector<row, column> negativeDivergences = RowVector<row, column>();
	RowVector<row, column> precon = RowVector<row, column>();

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

				gridData.emplace(GridDataPoint(Cell2D(render), GridDataPoint::FLUID));
			}
		}
	}

	double StandardPCG()
	{
		// PCG algorithm for solving Ap = b
		double pressureGuess = 0;

		RowVector<row, column> residualVector = negativeDivergences;
		RowVector<row, column> auxiliaryVector;

		applyPreconditioner(residualVector, auxiliaryVector);
		RowVector<row, column> searchVector = auxiliaryVector;

		double sigma = DotProduct(auxiliaryVector, residualVector);

		// TODO: Implement proper scaling
		double tolerance = 0.00001;

		for(unsigned int i = 0; i < 200; i++) // 200 here is max iterations
		{
			applyA(searchVector, auxiliaryVector);
			double alpha = sigma / DotProduct(auxiliaryVector, searchVector);
			pressureGuess = pressureGuess + alpha * sigma;
			residualVector = residualVector - (alpha * auxiliaryVector);

			if(residualVector.max() <= tolerance)
			{
				return pressureGuess;
			}

			applyPreconditioner(residualVector, auxiliaryVector);
			double sigmaNew = DotProduct(auxiliaryVector, residualVector);
			double beta = sigmaNew / sigma;

			searchVector = auxiliaryVector + (beta * searchVector);

			sigma = sigmaNew;
		}

		// Iteration limit exceeded
		return pressureGuess;
	}

	void applyA(const RowVector<row, column>& vector, RowVector<row, column>& result)
	{
		for(size_t i = 0; i < row; i++)
		{
			for(size_t j = 0; j < column; j++)
			{
				// This might be wrong
				result(i, j) = vector(i, j) * gridData(i, j).Adiag
					+ vector.at(i + 1, j) * gridData(i, j).Ax + vector.at(i, j + 1) * gridData(i, j).Ay;
			}
		}
	}

	/* Modified Incomplete Choleksy preconditoner to find a matrix M where M is approx A^ {-1}
	 Simple to implement, fairly efficient and robust in handling irregular domains (e.g liquid splash)
	 However hard to efficiently parallelize and not optimally scalable
	 */
	void applyPreconditioner(RowVector<row, column>& residualVector, RowVector<row, column>& result)
	{
		double tuningConstant = 0.97;
		double safetyConstant = 0.25;

		double e = 0;
		double cellPressureCoefficient = 0;
		double t = 0;
		for(size_t i = 0; i < row; i++)
		{
			for(size_t j = 0; j < column; j++)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					double Aplusi = gridData.at(i - 1, j).Ax;
					double Aplusj = gridData.at(i, j - 1).Ay;

					// NOTE: These will be the same as the precon from the last iteration of applyPreconditoner
					double previousXPrecon = precon.at(i - 1, j);
					double previousYPrecon = precon.at(i, j - 1);

					double cellPressureCoefficient = gridData(i, j).Adiag;

					e = cellPressureCoefficient - pow((Aplusi * previousXPrecon), 2)
						- pow((Aplusj - 1) * previousYPrecon, 2);

					e -= tuningConstant * (Aplusi * (Aplusj * pow(previousXPrecon, 2))
						+ Aplusj * (Aplusi * previousYPrecon, 2));

					if(e < safetyConstant * cellPressureCoefficient)
					{
						e = cellPressureCoefficient;
					}

					precon(i, j) = 1 / sqrt(e);

					// Solve Lq = r
					t = residualVector(i, j)
						- Aplusi * previousXPrecon * gridData.at(i - 1, j).q
						- Aplusj * previousYPrecon * gridData.at(i - 1, j - 1).q;

					gridData(i, j).q = t * precon(i, j);

				}
			}
		}

		for(size_t i = row - 1; i + 1 >= 1; i--)
		{
			for(size_t j = column - 1; j + 1 >= 1; j--)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					t = gridData(i, j).q
						- gridData(i, j).Ax * precon(i, j) * gridData.at(i + 1, j).z
						- gridData(i, j).Ay * precon(i, j) * gridData.at(i, j + 1).z;
				}

				gridData(i, j).z = t * precon(i, j);
			}
		}
		result = precon;
	}

	inline void UpdatePressure(float Acoefficient, GridDataPoint& cellData, GridDataPoint::CellState leftState, GridDataPoint::CellState rightState, GridDataPoint::CellState belowState, GridDataPoint::CellState aboveState)
	{
		if(leftState == GridDataPoint::FLUID) // Left neighbour
		{
			cellData.Adiag += Acoefficient;
		}

		if(rightState == GridDataPoint::FLUID) // Right neighbour
		{
			cellData.Adiag += Acoefficient;
			cellData.Ax = -Acoefficient;
		} else if(rightState == GridDataPoint::EMPTY)
		{
			cellData.Adiag += Acoefficient;
		}

		if(belowState == GridDataPoint::FLUID) // Below neighbour
		{
			cellData.Adiag += Acoefficient;
		}

		if(aboveState == GridDataPoint::FLUID) // Above neighbour
		{
			cellData.Adiag += Acoefficient;
			cellData.Ay = -Acoefficient;
		} else if(aboveState == GridDataPoint::EMPTY)
		{
			cellData.Adiag += Acoefficient;
		}
	}

	// TODO: A 1xn or nx1 grid is currently broken
	void Update(float timeStep)
	{
		float scale = 1 / (density * cellWidth);
		float Acoefficient = timeStep / (density * cellWidth * cellWidth);

		size_t i = 0;
		size_t j = 0;

		/* uVelocity in the GridDataPoint refers to the right u velocity arrow for that cell in the MAC Grid
		* Likewise vVelocity refers to the up v velocity arrow for that cell
		* Hence to get the arrow to the left or bottom we lookup the cell to the left or below the current cell respectively
		* These cells do not exist for the leftmost and bottommost cells so we have a special cases (the missing velocitys are set to 0)
		* Bottom left is an extra special case because it has neither a left arrow or bottom arrow
		*/

		// Bottom left
		negativeDivergences(0, 0) = -(gridData(0, 0).uVelocity + gridData(0, 0).vVelocity) / cellWidth;
		UpdatePressure(Acoefficient, gridData(0, 0), GridDataPoint::EMPTY, gridData(1, 0).cellState, GridDataPoint::EMPTY, gridData(0, 1).cellState);

		// Leftmost cells (except bottom left)
		for(j = 1; j < column; j++)
		{
			// i = 0
			negativeDivergences(0, j) = -(gridData(0, j).uVelocity + gridData(0, j).vVelocity - gridData(0, j - 1).vVelocity) / cellWidth;

			if(j == column - 1)
			{
				// Top left
				UpdatePressure(Acoefficient, gridData(0, j), GridDataPoint::EMPTY, gridData(1, j).cellState, gridData(0, j - 1).cellState, GridDataPoint::EMPTY);
			} else
			{
				UpdatePressure(Acoefficient, gridData(0, j), GridDataPoint::EMPTY, gridData(1, j).cellState, gridData(0, j - 1).cellState, gridData(0, j + 1).cellState);
			}
		}

		// Bottommost cells (except bottom left)
		for(i = 1; i < row; i++)
		{
			// j = 0
			negativeDivergences(i, 0) = -(gridData(i, 0).uVelocity + gridData(i - 1, 0).uVelocity + gridData(i, 0).vVelocity) / cellWidth;

			if(i == row - 1)
			{
				// Bottom right
				UpdatePressure(Acoefficient, gridData(i, 0), gridData(i - 1, 0).cellState, GridDataPoint::EMPTY, GridDataPoint::EMPTY, gridData(i, 1).cellState);
			} else
			{
				UpdatePressure(Acoefficient, gridData(i, 0), gridData(i - 1, 0).cellState, gridData(i + 1, 0).cellState, GridDataPoint::EMPTY, gridData(i, 1).cellState);
			}
		}


		// All other cells
		for(size_t i = 1; i < row; i++)
		{
			for(size_t j = 1; j < column; j++)
			{
				gridData(i, j).uVelocity -= timeStep * scale * gridData.at(i + 1, j).pressure - gridData(i, j).pressure;
				gridData(i, j).vVelocity -= timeStep * scale * gridData.at(i, j + 1).pressure - gridData(i, j).pressure;

				// TODO: remove division
				negativeDivergences(i, j) = -(gridData(i, j).uVelocity - gridData(i - 1, j).uVelocity +
					gridData(i, j).vVelocity - gridData(i, j - 1).vVelocity) / cellWidth;

				switch(gridData(i, j).cellState)
				{
					case GridDataPoint::SOLID:
						break;
					case GridDataPoint::FLUID:

						// Pressure coefficient update

						// We use 'at' for the positive i and j cells which will not exist for the topmost and rightmost cells
						UpdatePressure(Acoefficient, gridData(i, j), gridData(i - 1, j).cellState, gridData.at(i + 1, j).cellState,
							gridData(i, j - 1).cellState, gridData.at(i, j + 1).cellState);

						break;
					case GridDataPoint::EMPTY:
						break;
				}
			}
		}
		StandardPCG();
		GridDataPoint t = gridData(5, 5);
	}
};