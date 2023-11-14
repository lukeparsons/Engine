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
					//smoke(i, j) = smoke(originali, originalj);
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
			}
		}
	}
	//std::cout << "Number of same: " << count << std::endl;
}

void Grid2D::UpdateA(float Acoefficient, size_t i, size_t j)
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

// TODO: A 1xn or nx1 grid is currently broken
void Grid2D::Solve(float timeStep)
{
	float divergenceScale = 1.0f / cellWidth;
	float Acoefficient = timeStep / (density * cellWidth * cellWidth);

	size_t i = 0;
	size_t j = 0;

	/* uVelocity in the GridDataPoint refers to the right u velocity arrow for that cell in the MAC Grid
	* Likewise vVelocity refers to the up v velocity arrow for that cell
	*/

	for(size_t i = 0; i < column; i++)
	{
		for(size_t j = 0; j < row; j++)
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

	for(size_t i = 0; i < column; i++)
	{
		for(size_t j = 0; j < row; j++)
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