#include "Grid2D.h"

// data1 and data2 are temp
void Grid2D::advect(float timeStep, GridStructureHalo<float>& data1, GridStructureHalo<float>& data2)
{
	float scale = 1 / cellWidth;
	int count = 0;
	for(size_t i = 0; i < column; i++)
	{
		for(size_t j = 0; j < row; j++)
		{
			if(gridData(i, j).cellState == GridDataPoint::FLUID)
			{

				float x = i - timeStep * uVelocity(i, j) * scale;
				float y = j - timeStep * vVelocity(i, j) * scale;
				int originali = static_cast<int>(std::roundf(x));
				int originalj = static_cast<int>(std::roundf(y));

				//std::cout << "From (" << i << ", " << j << ") to (" << originali << ", " << originalj << ")" << std::endl;
				if(snap_to_grid(originali, originalj))
				{
					//std::cout << "snapped" << std::endl;
					data1(i, j) = data1(originali, originalj);
					data2(i, j) = data2(originali, originalj);
					smoke(i, j) = smoke(originali, originalj);
					continue;
				}
				// Interpolate

				float w01 = abs(x - originali) * cellWidth;
				float w11 = abs(y - originalj) * cellWidth;
				float w00 = 1 - w01;
				float w10 = 1 - w11;

				data1(i, j) = w00 * w10 * data1(originali, originalj)
					+ w01 * w10 * data1(originali + 1, originalj)
					+ w01 * w11 * data1(originali, originalj + 1)
					+ w00 * w11 * data1(originali + 1, originalj + 1);

				data2(i, j) = w00 * w10 * data2(originali, originalj)
					+ w01 * w10 * data2(originali + 1, originalj)
					+ w01 * w11 * data2(originali, originalj + 1)
					+ w00 * w11 * data2(originali + 1, originalj + 1);

				smoke(i, j) = w00 * w10 * smoke(originali, originalj)
					+ w01 * w10 * smoke(originali + 1, originalj)
					+ w01 * w11 * smoke(originali, originalj + 1)
					+ w00 * w11 * smoke(originali + 1, originalj + 1);

			}
		}
	}
	//std::cout << "Number of same: " << count << std::endl;
}