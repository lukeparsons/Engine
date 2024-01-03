#include "Grid2D.h"

#define MAX_ITERATIONS 100

// TODO: A 1xn or nx1 grid is currently broken
void Grid2D::PCGSolve(float timeStep)
{
	float divergenceScale = 1.0f / cellWidth;
	float Acoefficient = timeStep / (density * cellWidth * cellWidth);

	unsigned int i = 0;
	unsigned int j = 0;

	/* uVelocity in the GridDataPoint refers to the right u velocity arrow for that cell in the MAC Grid
	* Likewise vVelocity refers to the up v velocity arrow for that cell
	*/

	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{

			// Pressure coefficient update
			if(gridData(i, j).cellState == GridDataPoint2D::FLUID)
			{
				GridDataPoint2D& cellData = gridData(i, j);

				negativeDivergences(i, j) -= divergenceScale * (uVelocity(i + 1, j) - uVelocity(i, j) + vVelocity(i, j + 1) - vVelocity(i, j));

				GridDataPoint2D::CellState leftState = gridData(i - 1, j).cellState;
				if(leftState == GridDataPoint2D::FLUID) // Left neighbour
				{
					cellData.Adiag += Acoefficient;
				} else if(leftState == GridDataPoint2D::SOLID)
				{
					negativeDivergences(i, j) -= divergenceScale * (uVelocity(i, j) - 0); // usolid(i, j) 
				}

				GridDataPoint2D::CellState rightState = gridData(i + 1, j).cellState;
				if(rightState == GridDataPoint2D::FLUID) // Right neighbour
				{
					cellData.Adiag += Acoefficient;
					cellData.Ax = -Acoefficient;
				} else if(rightState == GridDataPoint2D::SOLID)
				{
					negativeDivergences(i, j) += divergenceScale * (uVelocity(i + 1, j) - 0); // usolid(i + 1, j)
				} else
				{
					cellData.Adiag += Acoefficient; // Right is empty state
				}

				GridDataPoint2D::CellState belowState = gridData(i, j - 1).cellState;
				if(belowState == GridDataPoint2D::FLUID) // Below neighbour
				{
					cellData.Adiag += Acoefficient;
				} else if(belowState == GridDataPoint2D::SOLID)
				{
					negativeDivergences(i, j) -= divergenceScale * (vVelocity(i, j) - 0); // vsolid(i, j)
				}

				GridDataPoint2D::CellState aboveState = gridData(i, j + 1).cellState;
				if(aboveState == GridDataPoint2D::FLUID) // Above neighbour
				{
					cellData.Adiag += Acoefficient;
					cellData.Ay = -Acoefficient;
				} else if(aboveState == GridDataPoint2D::SOLID)
				{
					negativeDivergences(i, j) += divergenceScale * (vVelocity(i, j + 1) - 0); // vsolid(i, j + 1)
				} else
				{
					cellData.Adiag += Acoefficient; // Above is empty
				}
			}
		}
	}

	PCG();

	float scale = timeStep / (density * cellWidth);

	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			switch(gridData(i, j).cellState)
			{
				case GridDataPoint2D::SOLID:
					if(gridData(i - 1, j).cellState == GridDataPoint2D::FLUID)
					{
						uVelocity(i, j) = 0; // usolid(i, j)
					} else
					{
						// Mark uVelocity(i, j) as unknown
					}

					if(gridData(i, j - 1).cellState == GridDataPoint2D::FLUID)
					{
						vVelocity(i, j) = 0; // vsolid(i, j)
					} else
					{
						// Mark vVelocity(i, j) as unknown
					}
					break;
				case GridDataPoint2D::FLUID:
					if(gridData(i - 1, j).cellState == GridDataPoint2D::SOLID)
					{
						uVelocity(i, j) = 0; // usolid(i, j)
					} else
					{
						uVelocity(i, j) -= scale * (pressure(i, j) - pressure(i - 1, j));
					}

					if(gridData(i, j - 1).cellState == GridDataPoint2D::SOLID)
					{
						vVelocity(i, j) = 0; // vsolid(i, j)
					} else
					{
						vVelocity(i, j) -= scale * (pressure(i, j) - pressure(i, j - 1));
					}
					break;
				case GridDataPoint2D::EMPTY:
					uVelocity(i, j) = 0;
					vVelocity(i, j) = 0;
					break;
			}
		}
	}
}


void Grid2D::PCG()
{
	if(std::all_of(negativeDivergences.begin(), negativeDivergences.end(), [](float val) { return val == 0; }))
	{
		return;
	}

	// PCG algorithm for solving Ap = b
	constructPreconditioner();
	pressure.fill(0); // Initial pressure guess


	RowVector2D residualVector = negativeDivergences;
	RowVector2D auxiliaryVector = RowVector2D(column, row);

	applyPreconditioner(residualVector, auxiliaryVector);
	RowVector2D searchVector = auxiliaryVector;

	double sigma = DotProduct(auxiliaryVector, residualVector);

	// TODO: Implement proper scaling
	double tolerance = 0.00001f;

	for(unsigned int iter = 0; iter < MAX_ITERATIONS; iter++)
	{
		applyA(searchVector, auxiliaryVector);
		double dP = DotProduct(auxiliaryVector, searchVector);
		double alpha = sigma / dP;

		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				pressure(i, j) += alpha * searchVector(i, j);
				residualVector(i, j) -= alpha * auxiliaryVector(i, j); 			//residualVector = residualVector - (alpha * auxiliaryVector);
			}
		}

		if(residualVector.max() <= tolerance)
		{
			std::cout << "Completed in " << iter << " iterations" << std::endl;
			return;
		}

		applyPreconditioner(residualVector, auxiliaryVector);
		double sigmaNew = DotProduct(auxiliaryVector, residualVector);
		double beta = sigmaNew / sigma;

		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				searchVector(i, j) = auxiliaryVector(i, j) + (beta * searchVector(i, j));
			}
		}

		sigma = sigmaNew;
	}

	// Iteration limit exceeded
	std::cout << "exceeded iterations" << std::endl;
}

void Grid2D::applyA(const RowVector2D& vector, RowVector2D& result)
{
	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			result(i, j) = gridData(i, j).Adiag * vector(i, j)
				+ gridData(i, j).Ax * vector(i + 1, j)
				+ gridData(i, j).Ay * vector(i, j + 1);
		}
	}
}

/* Modified Incomplete Choleksy preconditoner to find a matrix M where M is approx A^ {-1}
	Simple to implement, fairly efficient and robust in handling irregular domains (e.g liquid splash)
	However hard to efficiently parallelize and not optimally scalable
*/
void Grid2D::applyPreconditioner(RowVector2D& residualVector, RowVector2D& auxiliaryVector)
{
	double t = 0;

	RowVector2D q = RowVector2D(column, row);

	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			if(gridData(i, j).cellState == GridDataPoint2D::FLUID)
			{
				double prevIAx = gridData(i - 1, j).Ax;
				double prevIAy = gridData(i - 1, j).Ay;
				double prevJAx = gridData(i, j - 1).Ax;
				double prevJAy = gridData(i, j - 1).Ay;

				// Solve Lq = r
				t = residualVector(i, j) - prevIAx * precon(i - 1, j) * q(i - 1, j)
					- prevJAy * precon(i, j - 1) * q(i, j - 1);

				q(i, j) = t * precon(i, j);
			}
		}
	}

	for(unsigned int i = column - 1; i-- > 0;)
	{
		for(unsigned int j = row - 1; j-- > 0;)
		{
			if(gridData(i, j).cellState == GridDataPoint2D::FLUID)
			{
				t = q(i, j) - gridData(i, j).Ax * precon(i, j) * auxiliaryVector(i + 1, j)
					- gridData(i, j).Ay * precon(i, j) * auxiliaryVector(i, j + 1);
			}

			auxiliaryVector(i, j) = t * precon(i, j);
		}
	}
}

void Grid2D::constructPreconditioner()
{
	double tuningConstant = 0.97;
	double safetyConstant = 0.25;

	double e = 0;

	precon.fill(0);
	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			if(gridData(i, j).cellState == GridDataPoint2D::FLUID)
			{
				double prevIAx = gridData(i - 1, j).Ax;
				double prevIAy = gridData(i - 1, j).Ay;
				double prevJAx = gridData(i, j - 1).Ax;
				double prevJAy = gridData(i, j - 1).Ay;

				double Adiag = gridData(i, j).Adiag;

				e = Adiag - pow(prevIAx * precon(i - 1, j), 2)
					- pow(prevJAy * precon(i, j - 1), 2)
					- tuningConstant * (prevIAx * prevIAy * pow(precon(i - 1, j), 2)
						+ prevJAy * (prevJAx * pow(precon(i, j - 1), 2)));


				if(e < safetyConstant * Adiag)
				{
					e = Adiag;
				}

				precon(i, j) = 1 / sqrt(e);
			}
		}
	}
}