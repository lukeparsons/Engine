#include "Grid3D.h"

#define MAX_ITERATIONS 200

// TODO: A 1xn or nx1 grid is currently broken
void Grid3D::PCGSolve(float timeStep)
{

	float scale = timeStep / (density * cellWidth);

	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				switch(gridData(i, j, k).cellState)
				{
					case GridDataPoint::SOLID:
						if(gridData(i - 1, j, k).cellState == GridDataPoint::FLUID)
						{
							uVelocity(i, j, k) = 0; // usolid(i, j)
						} else
						{
							// Mark uVelocity(i, j) as unknown
						}

						if(gridData(i, j - 1, k).cellState == GridDataPoint::FLUID)
						{
							vVelocity(i, j, k) = 0; // vsolid(i, j)
						} else
						{
							// Mark vVelocity(i, j) as unknown
						}

						if(gridData(i, j, k - 1).cellState == GridDataPoint::FLUID)
						{
							wVelocity(i, j, k) = 0; // vsolid(i, j)
						} else
						{
							// Mark wVelocity(i, j) as unknown
						}
						break;
					case GridDataPoint::FLUID:
						if(gridData(i - 1, j, k).cellState == GridDataPoint::SOLID)
						{
							uVelocity(i, j, k) = 0; // usolid(i, j)
						} else
						{
							uVelocity(i, j, k) -= scale * (pressure(i, j, k) - pressure(i - 1, j, k));
						}

						if(gridData(i, j - 1, k).cellState == GridDataPoint::SOLID)
						{
							vVelocity(i, j, k) = 0; // vsolid(i, j)
						} else
						{
							vVelocity(i, j, k) -= scale * (pressure(i, j, k) - pressure(i, j - 1, k));
						}

						if(gridData(i, j - 1, k - 1).cellState == GridDataPoint::SOLID)
						{
							wVelocity(i, j, k) = 0; // vsolid(i, j)
						} else
						{
							wVelocity(i, j, k) -= scale * (pressure(i, j, k) - pressure(i, j, k - 1));
						}
						break;
					case GridDataPoint::EMPTY:
						break;
				}
			}

		}
	}

	float divergenceScale = 1.0f / cellWidth;

	float Acoefficient = timeStep / (density * cellWidth * cellWidth);

	unsigned int i = 0;
	unsigned int j = 0;

	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				// Pressure coefficient update
				if(gridData(i, j, k).cellState == GridDataPoint::FLUID)
				{

					GridDataPoint& cellData = gridData(i, j, k);

					negativeDivergences(i, j, k) -= divergenceScale * (uVelocity(i + 1, j, k) - uVelocity(i, j, k) 
						+ vVelocity(i, j + 1, k) - vVelocity(i, j, k)
						+ wVelocity(i, j, k + 1) - wVelocity(i, j, k));

					GridDataPoint::CellState leftState = gridData(i - 1, j, k).cellState;
					if(leftState == GridDataPoint::FLUID) // Left neighbour
					{
						cellData.Adiag += Acoefficient;
					} else if(leftState == GridDataPoint::SOLID)
					{
						negativeDivergences(i, j, k) -= divergenceScale * (uVelocity(i, j, k) - 0); // usolid(i, j) 
					}

					GridDataPoint::CellState rightState = gridData(i + 1, j, k).cellState;
					if(rightState == GridDataPoint::FLUID) // Right neighbour
					{
						cellData.Adiag += Acoefficient;
						cellData.Ax = -Acoefficient;
					} else if(rightState == GridDataPoint::SOLID)
					{
						negativeDivergences(i, j, k) += divergenceScale * (uVelocity(i + 1, j, k) - 0); // usolid(i + 1, j)
					} else
					{
						cellData.Adiag += Acoefficient; // Right is empty state
					}

					GridDataPoint::CellState belowState = gridData(i, j - 1, k).cellState;
					if(belowState == GridDataPoint::FLUID) // Below neighbour
					{
						cellData.Adiag += Acoefficient;
					} else if(belowState == GridDataPoint::SOLID)
					{
						negativeDivergences(i, j, k) -= divergenceScale * (vVelocity(i, j, k) - 0); // vsolid(i, j)
					}

					GridDataPoint::CellState aboveState = gridData(i, j + 1, k).cellState;
					if(aboveState == GridDataPoint::FLUID) // Above neighbour
					{
						cellData.Adiag += Acoefficient;
						cellData.Ay = -Acoefficient;
					} else if(aboveState == GridDataPoint::SOLID)
					{
						negativeDivergences(i, j, k) += divergenceScale * (vVelocity(i, j + 1, k) - 0); // vsolid(i, j + 1)
					} else
					{
						cellData.Adiag += Acoefficient; // Above is empty
					}

					GridDataPoint::CellState behindState = gridData(i, j, k - 1).cellState;
					if(behindState == GridDataPoint::FLUID) // Behind neighbour
					{
						cellData.Adiag += Acoefficient;
					} else if(behindState == GridDataPoint::SOLID)
					{
						negativeDivergences(i, j, k) -= divergenceScale * (wVelocity(i, j, k) - 0); // wsolid()
					}

					GridDataPoint::CellState infrontState = gridData(i, j, k + 1).cellState;
					if(infrontState == GridDataPoint::FLUID) // In front neighbour
					{
						cellData.Adiag += Acoefficient;
						cellData.Az = -Acoefficient;
					} else if(infrontState == GridDataPoint::SOLID)
					{
						negativeDivergences(i, j, k) += divergenceScale * (wVelocity(i, j, k + 1) - 0); // wsolid
					} else
					{
						cellData.Adiag += Acoefficient; // Above is empty
					}
				}
			}
		}
	}

	PCG();
}


void Grid3D::PCG()
{
	if(std::all_of(negativeDivergences.begin(), negativeDivergences.end(), [](float val) { return val == 0; }))
	{
		return;
	}

	// PCG algorithm for solving Ap = b
	constructPreconditioner();
	pressure.fill(0); // Initial pressure guess


	RowVector residualVector = negativeDivergences;
	RowVector auxiliaryVector = RowVector(column, row, depth);

	applyPreconditioner(residualVector, auxiliaryVector);
	RowVector searchVector = auxiliaryVector;

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
				for(unsigned int k = 0; k < depth; k++)
				{
					pressure(i, j, k) += alpha * searchVector(i, j, k);
					residualVector(i, j, k) -= alpha * auxiliaryVector(i, j, k); 			//residualVector = residualVector - (alpha * auxiliaryVector);
				}

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
				for(unsigned int k = 0; k < depth; k++)
				{
					searchVector(i, j, k) = auxiliaryVector(i, j, k) + (beta * searchVector(i, j, k));
				}
			}
		}

		sigma = sigmaNew;
	}

	// Iteration limit exceeded
	std::cout << "exceeded iterations" << std::endl;
}

void Grid3D::applyA(const RowVector& vector, RowVector& result)
{
	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				result(i, j, k) = gridData(i, j, k).Adiag * vector(i, j, k)
					+ gridData(i, j, k).Ax * vector(i + 1, j, k)
					+ gridData(i, j, k).Ay * vector(i, j + 1, k)
					+ gridData(i, j, k).Az * vector(i, j, k + 1);
			}

		}
	}
}

/* Modified Incomplete Choleksy preconditoner to find a matrix M where M is approx A^ {-1}
	Simple to implement, fairly efficient and robust in handling irregular domains (e.g liquid splash)
	However hard to efficiently parallelize and not optimally scalable
*/
void Grid3D::applyPreconditioner(RowVector& residualVector, RowVector& auxiliaryVector)
{
	double t = 0;

	RowVector q = RowVector(column, row, depth);

	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				if(gridData(i, j, k).cellState == GridDataPoint::FLUID)
				{
					double prevIAx = gridData(i - 1, j, k).Ax;
					double prevIAy = gridData(i - 1, j, k).Ay;
					double prevIAz = gridData(i - 1, j, k).Az;
					double prevJAx = gridData(i, j - 1, k).Ax;
					double prevJAy = gridData(i, j - 1, k).Ay;
					double prevJAz = gridData(i, j - 1, k).Az;
					double prevKAx = gridData(i, j, k - 1).Ax;
					double prevKAy = gridData(i, j, k - 1).Ay;
					double prevKAz = gridData(i, j, k - 1).Az;

					// Solve Lq = r
					t = residualVector(i, j, k) - prevIAx * precon(i - 1, j, k) * q(i - 1, j, k)
						- prevJAy * precon(i, j - 1, k) * q(i, j - 1, k)
						- prevKAz * precon(i, j, k - 1) * q(i, j, k - 1);

					q(i, j, k) = t * precon(i, j, k);
				}
			}

		}
	}

	for(unsigned int i = column - 1; i-- > 0;)
	{
		for(unsigned int j = row - 1; j-- > 0;)
		{
			for(unsigned int k = depth - 1; k-- > 0;)
			{
				if(gridData(i, j, k).cellState == GridDataPoint::FLUID)
				{
					t = q(i, j, k) - gridData(i, j, k).Ax * precon(i, j, k) * auxiliaryVector(i + 1, j, k)
						- gridData(i, j, k).Ay * precon(i, j, k) * auxiliaryVector(i, j + 1, k)
						- gridData(i, j, k).Az * precon(i, j, k) * auxiliaryVector(i, j, k + 1);
				}

				auxiliaryVector(i, j, k) = t * precon(i, j, k);
			}

		}
	}
}

void Grid3D::constructPreconditioner()
{
	double tuningConstant = 0.97;
	double safetyConstant = 0.25;

	double e = 0;

	precon.fill(0);
	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				if(gridData(i, j, k).cellState == GridDataPoint::FLUID)
				{
					double prevIAx = gridData(i - 1, j, k).Ax;
					double prevIAy = gridData(i - 1, j, k).Ay;
					double prevIAz = gridData(i - 1, j, k).Az;
					double prevJAx = gridData(i, j - 1, k).Ax;
					double prevJAy = gridData(i, j - 1, k).Ay;
					double prevJAz = gridData(i, j - 1, k).Az;
					double prevKAx = gridData(i, j, k - 1).Ax;
					double prevKAy = gridData(i, j, k - 1).Ay;
					double prevKAz = gridData(i, j, k - 1).Az;

					double Adiag = gridData(i, j, k).Adiag;

					e = Adiag - pow(prevIAx * precon(i - 1, j, k), 2)
						- pow(prevJAy * precon(i, j - 1, k), 2)
						- pow(prevJAz * precon(i, j, k - 1), 2)
						- tuningConstant * (prevIAx * (prevIAy + prevIAz) * pow(precon(i - 1, j, k), 2)
							+ prevJAy * (prevJAx + prevJAz) * pow(precon(i, j - 1, k), 2)
							+ prevKAz * (prevKAx + prevKAy) * pow(precon(i, j, k - 1), 2));

					if(e < safetyConstant * Adiag)
					{
						e = Adiag;
					}

					precon(i, j, k) = 1 / sqrt(e);
				}
			}
		}
	}
}