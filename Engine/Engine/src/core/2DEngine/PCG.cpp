#include "Grid2D.h"

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

			switch(gridData(i, j).cellState)
			{
				case GridDataPoint::SOLID:
					// TODO: Remember to update Adiag, Ax etc in a ChangeState function
					/*if(gridData(i - 1, j).cellState == FLUID)
					{
						// pg 71
					} */

					break;
				case GridDataPoint::FLUID:

					// Pressure coefficient update

					// TODO: account for solid velocities
					negativeDivergences(i, j) -= divergenceScale * (uVelocity(i + 1, j) - uVelocity(i, j) + vVelocity(i, j + 1) - vVelocity(i, j));
					UpdateA(Acoefficient, i, j);
					break;
				case GridDataPoint::EMPTY:
					// Assuming empty fluid cell pressure = 0
					break;
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
				case GridDataPoint::SOLID:
					if(gridData(i - 1, j).cellState == GridDataPoint::FLUID)
					{
						uVelocity(i, j) = 0; // usolid(i, j
					} else
					{
						// Mark uVelocity(i, j) as unknown
					}

					if(gridData(i, j - 1).cellState == GridDataPoint::FLUID)
					{
						vVelocity(i, j) = 0; // vsolid(i, j)
					} else
					{
						// Mark vVelocity(i, j) as unknown
					}
					break;
				case GridDataPoint::FLUID:
					if(gridData(i - 1, j).cellState == GridDataPoint::SOLID)
					{
						uVelocity(i, j) = 0; // usolid(i, j)
					} else
					{
						uVelocity(i, j) -= scale * (pressure(i, j) - pressure(i - 1, j));
					}

					if(gridData(i, j - 1).cellState == GridDataPoint::SOLID)
					{
						vVelocity(i, j) = 0; // vsolid(i, j)
					} else
					{
						vVelocity(i, j) -= scale * (pressure(i, j) - pressure(i, j - 1));
					}
					break;
				case GridDataPoint::EMPTY:
					if(gridData(i - 1, j).cellState == GridDataPoint::FLUID)
					{
						uVelocity(i, j) -= scale * (pressure(i, j) - pressure(i - 1, j));
					} else
					{
						// Mark uVelocity(i, j) as unknown
					}

					if(gridData(i, j - 1).cellState == GridDataPoint::FLUID)
					{
						vVelocity(i, j) -= scale * (pressure(i, j) - pressure(i, j - 1));
					} else
					{
						// Mark vVelocity(i, j) as unknown
					}
			}
		}
	}
}

void Grid2D::UpdateA(float Acoefficient, unsigned int i, unsigned int j)
{

	GridDataPoint& cellData = gridData(i, j);

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
void Grid2D::PCG()
{
	// PCG algorithm for solving Ap = b
	pressure.fill(0); // Pressure guess
	constructPreconditioner();

	if(std::all_of(negativeDivergences.begin(), negativeDivergences.end(), [](float val) { return val == 0; }))
	{
		return;
	}

	RowVector residualVector = negativeDivergences;
	RowVector auxiliaryVector = RowVector(column, row);

	applyPreconditioner(residualVector, auxiliaryVector);
	RowVector searchVector = auxiliaryVector;

	double sigma = DotProduct(auxiliaryVector, residualVector);

	// TODO: Implement proper scaling
	double tolerance = 0.000001f;

	for(unsigned int iter = 0; iter < 200; iter++) // 200 here is max iterations
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

void Grid2D::applyA(const RowVector& vector, RowVector& result)
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
void Grid2D::applyPreconditioner(RowVector& residualVector, RowVector& auxiliaryVector)
{
	double t = 0;

	RowVector q = RowVector(column, row);

	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			if(gridData(i, j).cellState == GridDataPoint::FLUID)
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
			if(gridData(i, j).cellState == GridDataPoint::FLUID)
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
			if(gridData(i, j).cellState == GridDataPoint::FLUID)
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