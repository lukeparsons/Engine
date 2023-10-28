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

	GridStructure<GridDataPoint, row, column> gridData;
	RowVector<row, column> negativeDivergences;
	RowVector<row, column> precon;

	Grid2D(Scene& scene, std::shared_ptr<Mesh>& gridModel, const Vector2f& location, float _density, float _cellWidth) : density(_density), cellWidth(_cellWidth)
	{
		Vector3f cellScale = Vector3f(cellWidth, cellWidth, cellWidth);

		for(unsigned int i = 1; i <= row; i++)
		{
			for(unsigned int j = 1; j <= column; j++)
			{
				EntityID cellID = scene.CreateModel(gridModel, solidTexture,
					Vector3f(location.x, location.y, 0) + Vector3f(i * cellWidth * 2, j * cellWidth * 2, 0), cellScale);
				RenderComponent* render = scene.GetComponent<RenderComponent>(cellID);

				gridData.emplace(GridDataPoint(Cell2D(render), GridDataPoint::FLUID), i, j);
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

		// Bottom left corner
		result(1, 1) = gridData(1, 1).Adiag * vector(1, 1)
			+ gridData(1, 1).Ax * vector(2, 1);
			+ gridData(1, 1).Ay * vector(1, 2);
		
		// Bottom right corner
		result(column, 1) = gridData(column, 1).Adiag * vector(column, 1)
			+ gridData(column - 1, 1).Ax * vector(column - 1, 1)
			+ gridData(column, 1).Ay * vector(column, 2);

		// Top left corner
		result(1, row) = gridData(1, row).Adiag * vector(1, row)
			+ gridData(1, row).Ax * vector(2, row)
			+ gridData(1, row - 1).Ay * vector(1, row - 1);

		// Top right corner
		result(column, row) = gridData(column, row).Adiag * vector(column, row)
			+ gridData(column - 1, row).Ax * vector(column - 1, row)
			+ gridData(column, row - 1).Ay * vector(column, row - 1);

		// Top and bottom cells
		for(size_t i = 2; i < column; i++)
		{
			result(i, 1) = gridData(i, 1).Adiag * vector(i, 1)
				+ gridData(i - 1, 1).Ax * vector(i - 1, 1)
				+ gridData(i, 1).Ax * vector(i + 1, 1)
				+ gridData(i, 1).Ay * vector(i, 2);

			result(i, row) = gridData(i, row).Adiag * vector(i, row)
				+ gridData(i - 1, row).Ax * vector(i - 1, row)
				+ gridData(i, row).Ax * vector(i + 1, row)
				+ gridData(i, row - 1).Ay * vector(i, row - 1);
		}

		// Left and right cells
		for(size_t j = 2; j < row; j++)
		{
			result(1, j) = gridData(1, j).Adiag * vector(1, j)
				+ gridData(1, j).Ax * vector(2, j)
				+ gridData(1, j).Ay * vector(1, j + 1)
				+ gridData(1, j - 1).Ay * vector(1, j - 1);

			result(column, j) = gridData(column, j).Adiag * vector(column, j)
				+ gridData(column - 1, j).Ax * vector(column - 1, j)
				+ gridData(column, j).Ay * vector(column, j + 1)
				+ gridData(column, j - 1).Ay * vector(column, j - 1);
		}

		// Internal cells
		for(size_t i = 2; i < row - 1; i++)
		{
			for(size_t j = 2; j <= column - 1; j++)
			{
				result(i, j) = gridData(i, j).Adiag * vector(i, j)
					+ gridData(i - 1, j).Ax * vector(i - 1, j)
					+ gridData(i, j).Ax * vector(i + 1, j)
					+ gridData(i, j).Ay * vector(i, j + 1)
					+ gridData(i, j - 1).Ay * vector(i, j - 1);
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

		double previousXPrecon = 0;
		double previousYPrecon = 0;
		for(size_t i = 1; i <= row; i++)
		{
			for(size_t j = 1; j <= column; j++)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					double Aplusi = gridData(i - 1, j).Ax;
					double Aplusj = gridData(i, j - 1).Ay;

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
						- Aplusi * previousXPrecon * gridData(i - 1, j).q
						- Aplusj * previousYPrecon * gridData(i - 1, j - 1).q;

					gridData(i, j).q = t * precon(i, j);

					// NOTE: These will be the same as the precon from the last iteration of applyPreconditoner
					double previousXPrecon = precon(i, j);
					double previousYPrecon = precon(i, j);
				}
			}
		}

		for(size_t i = row; i >= 1; i--)
		{
			for(size_t j = column; j >= 1; j--)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					t = gridData(i, j).q
						- gridData(i, j).Ax * precon(i, j) * gridData(i + 1, j).z
						- gridData(i, j).Ay * precon(i, j) * gridData(i, j + 1).z;
				}

				gridData(i, j).z = t * precon(i, j);
			}
		}
		result = precon;
	}

	inline void UpdatePressure(float Acoefficient, GridDataPoint& cellData, size_t i, size_t j)
	{
		if(gridData(i - 1, j).cellState == GridDataPoint::FLUID) // Left neighbour
		{
			cellData.Adiag += Acoefficient;
		}

		GridDataPoint::CellState rightState = gridData(i + 1, j).cellState;
		if(rightState == GridDataPoint::FLUID) // Right neighbour
		{
			cellData.Adiag += Acoefficient;
			cellData.Ax = -Acoefficient;
		} else if(rightState == GridDataPoint::EMPTY)
		{
			cellData.Adiag += Acoefficient;
		}

		if(gridData(i, j - 1).cellState == GridDataPoint::FLUID) // Below neighbour
		{
			cellData.Adiag += Acoefficient;
		}

		GridDataPoint::CellState aboveState = gridData(i, j + 1).cellState;
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
		*/

		for(size_t i = 1; i <= row; i++)
		{
			for(size_t j = 1; j <= column; j++)
			{

				gridData(i, j).uVelocity -= timeStep * scale * gridData(i + 1, j).pressure - gridData(i, j).pressure;
				gridData(i, j).vVelocity -= timeStep * scale * gridData(i, j + 1).pressure - gridData(i, j).pressure;

				// TODO: remove division
				negativeDivergences(i, j) = -(gridData(i, j).uVelocity - gridData(i - 1, j).uVelocity +
					gridData(i, j).vVelocity - gridData(i, j - 1).vVelocity) / cellWidth;

				switch(gridData(i, j).cellState)
				{
					case GridDataPoint::SOLID:
						// TODO: Remember to update Adiag, Ax etc in a ChangeState function
						break;
					case GridDataPoint::FLUID:

						// Pressure coefficient update

						UpdatePressure(Acoefficient, gridData(i, j), i, j);

						break;
					case GridDataPoint::EMPTY:
						break;
				}
			}
		}
		StandardPCG();
	}
};