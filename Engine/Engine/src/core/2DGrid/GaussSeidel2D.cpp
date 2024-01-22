#include "Grid2D.h"

void Grid2D::GaussSeidel(float timeStep)
{
	float overRelaxationScalar = 1.9;
	float pressureScale = density * cellWidth / timeStep;
	pressure.fill(0);
	for(unsigned int iter = 0; iter < 100; iter++) // 100 here is max iterations
	{
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{	
				if(gridData(i, j).cellState == GridDataPoint2D::FLUID)
				{

					int numFluidNeighbours = 0;

					int leftS = 0;
					int rightS = 0;
					int downS = 0;
					int upS = 0;

					if(gridData(i - 1, j).cellState == GridDataPoint2D::FLUID)
					{
						leftS = 1;
					}
					
					if(gridData(i + 1, j).cellState == GridDataPoint2D::FLUID)
					{
						rightS = 1;
					}

					if(gridData(i, j - 1).cellState == GridDataPoint2D::FLUID)
					{
						downS = 1;
					}

					if(gridData(i, j + 1).cellState == GridDataPoint2D::FLUID)
					{
						upS = 1;
					}
					
					numFluidNeighbours = leftS + rightS + downS + upS;

					if(numFluidNeighbours == 0)
					{
						continue;
					}

					float d = uVelocity(i + 1, j) - uVelocity(i, j) + vVelocity(i, j + 1) - vVelocity(i, j);

					// Overrelaxation
					float p = -d * overRelaxationScalar / numFluidNeighbours;
					pressure(i, j) += p * pressureScale;

					uVelocity(i, j) -= leftS * p;
					uVelocity(i + 1, j) += rightS * p;
					vVelocity(i, j) -= downS * p;
					vVelocity(i, j + 1) += upS * p;

				}
			}
		}
	}
}