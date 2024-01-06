#include "Grid3D.h"
#include <array>
#include <algorithm>

void Grid3D::clamp_to_grid(float x, float y, float z, unsigned int& i, unsigned int& j, unsigned int& k)
{
	int xGrid = std::roundf(x * scaleCellWidth);

	if(xGrid < 0)
	{
		i = 0;
	} else if(xGrid > column - 1)
	{
		i = column - 1;
	} else
	{
		i = xGrid;
	}

	int yGrid = std::roundf(y * scaleCellWidth);

	if(yGrid < 0)
	{
		j = 0;
	} else if(yGrid > row - 1)
	{
		j = row - 1;
	} else
	{
		j = yGrid;
	}

	int kGrid = std::roundf(z * scaleCellWidth);

	if(kGrid < 0)
	{
		k = 0;
	} else if(kGrid > row - 1)
	{
		k = row - 1;
	} else
	{
		k = yGrid;
	}

}

static float uVelocityAtPoint(GridStructureHalo<float>& uVelocity, unsigned int i, unsigned int j, unsigned int k)
{
	return (uVelocity(i - 1, j, k) + uVelocity(i, j, k)) / 2.0f;
}

static float vVelocityAtPoint(GridStructureHalo<float>& vVelocity, unsigned int i, unsigned int j, unsigned int k)
{
	return (vVelocity(i, j - 1, k) + vVelocity(i, j, k)) / 2.0f;
}

static float wVelocityAtPoint(GridStructureHalo<float>& wVelocity, unsigned int i, unsigned int j, unsigned int k)
{
	return (wVelocity(i, j, k - 1) + wVelocity(i, j, k)) / 2.0f;
}

static float cubicInterpolate(std::array<float, 4> axis, float x)
{
	return axis[1] + 0.5f * x * (axis[2] - axis[0] + x * (2.0f * axis[0] - 5.0f * axis[1] + 4.0f * axis[2] - axis[3] + x * (3.0f * (axis[1] - axis[2]) + axis[3] - axis[0])));
}

static float bicubicInterpolate(GridStructureHalo<float>& grid, unsigned int i, unsigned int j, unsigned int k, float cellWidth)
{
	float x = i * cellWidth;
	float y = j * cellWidth;
	std::array<float, 4> row;
	row[0] = cubicInterpolate({ grid(i - 1, j - 1, k), grid(i, j - 1, k), grid(i + 1, j - 1, k), grid(i + 2, j - 1, k) }, y);
	row[1] = cubicInterpolate({ grid(i - 1, j, k), grid(i, j, k), grid(i + 1, j, k), grid(i + 2, j, k) }, y);
	row[2] = cubicInterpolate({ grid(i - 1, j + 1, k), grid(i, j + 1, k), grid(i + 1, j + 1, k), grid(i + 2, j + 1, k) }, y);
	row[3] = cubicInterpolate({ grid(i - 1, j + 2, k), grid(i, j + 2, k), grid(i + 1, j + 2, k), grid(i + 2, j + 2, k) }, y);
	return cubicInterpolate(row, x);
}

// TODO: Tricubic interpolate

void Grid3D::advect(float timeStep)
{

	/*uVelocity.haloCondition();
	vVelocity.haloCondition();
	pressure.haloCondition();
	smoke.haloCondition(); */

	// RK-2
	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < row; k++)
			{
				float xMid = i * cellWidth - 0.5f * timeStep * uVelocityAtPoint(uVelocity, i, j, k);
				float yMid = j * cellWidth - 0.5f * timeStep * vVelocityAtPoint(vVelocity, i, j, k);
				float zMid = k * cellWidth - 0.5f * timeStep * wVelocityAtPoint(wVelocity, i, j, k);

				unsigned int iMid, jMid, kMid;
				clamp_to_grid(xMid, yMid, zMid, iMid, jMid, kMid);

				// Interpolate velocity fields at mid point
				float uMid = bicubicInterpolate(uVelocity, iMid, jMid, kMid, cellWidth);
				float vMid = bicubicInterpolate(vVelocity, iMid, jMid, kMid, cellWidth);
				float wMid = bicubicInterpolate(wVelocity, iMid, jMid, kMid, cellWidth);

				float xFinal = i * cellWidth - timeStep * uMid;
				float yFinal = j * cellWidth - timeStep * vMid;
				float zFinal = k * cellWidth - timeStep * wMid;
				unsigned int iFinal, jFinal, kFinal;
				clamp_to_grid(xFinal, yFinal, zFinal, iFinal, jFinal, kFinal);

				if(gridData(iFinal, jFinal, kFinal).cellState == GridDataPoint::SOLID)
				{
					continue;
				}

				uVelocity(i, j, k) = bicubicInterpolate(uVelocity, iFinal, jFinal, kFinal, cellWidth);
				vVelocity(i, j, k) = bicubicInterpolate(vVelocity, iFinal, jFinal, kFinal, cellWidth);
				vVelocity(i, j, k) = bicubicInterpolate(wVelocity, iFinal, jFinal, kFinal, cellWidth);
				smoke(i, j, k) = bicubicInterpolate(smoke, iFinal, jFinal, kFinal, cellWidth);
			}


		}
	}
	extrapolate();

}