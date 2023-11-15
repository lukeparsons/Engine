#include "Grid2D.h"

void Grid2D::GaussSeidel(float timeStep)
{
	float overRelaxationScalar = 1.9;
	float pressureScale = density * cellWidth / timeStep;
	for(unsigned int iter = 0; iter < 100; iter++) // 100 here is max iterations
	{
		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{	
				if(smoke(i, j) == 0)
				{
					continue;
				}

				if(gridData(i, j).cellState != GridDataPoint::SOLID)
				{

					float d = uVelocity(i + 1, j) - uVelocity(i, j) + vVelocity(i, j + 1) - vVelocity(i, j);

					float rightS = smoke(i + 1, j);
					float leftS = smoke(i - 1, j);
					float upS = smoke(i, j + 1);
					float downS = smoke(i, j - 1);

					float s = rightS + leftS + upS + downS;

					if(s == 0)
					{
						continue;
					}

					float scale = 1 / s;

					// TODO: Try measuring pressure step

					// Overrelaxation
					float p = -d * scale * overRelaxationScalar;
					pressure(i, j) += p * pressureScale;

					uVelocity(i, j) += leftS * p;
					uVelocity(i + 1, j) -= rightS * p;
					vVelocity(i, j) += downS * scale;
					vVelocity(i, j + 1) -= upS * scale;

				}
			}
		}
	}
}