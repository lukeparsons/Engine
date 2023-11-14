#include "Grid2D.h"

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

		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
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

		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
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
	for(size_t i = 0; i < column; i++)
	{
		for(size_t j = 0; j < row; j++)
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

	for(size_t i = 0; i < column; i++)
	{
		for(size_t j = 0; j < row; j++)
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

	for(size_t i = column - 1; i-- > 0;)
	{
		for(size_t j = row - 1; j-- > 0;)
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
	for(size_t i = 0; i < column; i++)
	{
		for(size_t j = 0; j < row; j++)
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