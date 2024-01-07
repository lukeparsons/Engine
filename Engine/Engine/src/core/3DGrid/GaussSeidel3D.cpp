#include "Grid3D.h"

void Grid3D::GaussSeidel(float timeStep)
{
	float overRelaxationScalar = 1.9;
	float pressureScale = density * cellWidth / timeStep;
	pressure.fill(0);
	for(unsigned int iter = 0; iter < 20; iter++) // 100 here is max iterations
	{
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{	
				for(unsigned int k = 0; k < depth; k++)
				{
					if(gridData(i, j, k).cellState == GridDataPoint::FLUID)
					{

						int numFluidNeighbours = 0;

						int leftS = 0;
						int rightS = 0;
						int downS = 0;
						int upS = 0;
						int behindS = 0;
						int frontS = 0;

						if(gridData(i - 1, j, k).cellState == GridDataPoint::FLUID)
						{
							leftS = 1;
						}

						if(gridData(i + 1, j, k).cellState == GridDataPoint::FLUID)
						{
							rightS = 1;
						}

						if(gridData(i, j - 1, k).cellState == GridDataPoint::FLUID)
						{
							downS = 1;
						}

						if(gridData(i, j + 1, k).cellState == GridDataPoint::FLUID)
						{
							upS = 1;
						}

						if(gridData(i, j, k + 1).cellState == GridDataPoint::FLUID)
						{
							frontS = 1;
						}

						if(gridData(i, j, k - 1).cellState == GridDataPoint::FLUID)
						{
							behindS = 1;
						}

						numFluidNeighbours = leftS + rightS + downS + upS + frontS + behindS;

						if(numFluidNeighbours == 0)
						{
							continue;
						}

						float d = uVelocity.get(i + 1, j, k) - uVelocity.get(i, j, k) + vVelocity.get(i, j + 1, k) - vVelocity.get(i, j, k) + wVelocity.get(i, j, k + 1) - wVelocity.get(i, j, k);

						// Overrelaxation
						float p = -d * overRelaxationScalar / numFluidNeighbours;
						pressure(i, j, k) += p * pressureScale;

						uVelocity(i, j, k) -= leftS * p;
						uVelocity(i + 1, j, k) += rightS * p;
						vVelocity(i, j, k) -= downS * p;
						vVelocity(i, j + 1, k) += upS * p;
						wVelocity(i, j, k) -= behindS * p;
						wVelocity(i, j, k + 1) += frontS * p;

					}
				}
			}
		}
	}
}