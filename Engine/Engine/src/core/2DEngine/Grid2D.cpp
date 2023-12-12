#include "Grid2D.h"
#include <array>
#include <algorithm>

bool Grid2D::clamp_to_grid(float x, float y, unsigned int& i, unsigned int& j)
{
	int xGrid = std::roundf(x * scaleCellWidth);
	bool snapped = false;

	if(xGrid < 0)
	{
		i = 0;
		snapped = true;
	} else if(xGrid > column - 1)
	{
		i = column - 1;
		snapped = true;
	} else
	{
		i = xGrid;
	}

	int yGrid = std::roundf(y * scaleCellWidth);
	
	if(yGrid < 0)
	{
		j = 0;
		return true;
	} else if(yGrid > row - 1)
	{
		j = row - 1;
		return true;
	} else
	{
		j = yGrid;
	}
	
	return snapped;
}

static float uVelocityAtPoint(GridStructureHalo<float>& uVelocity, unsigned int i, unsigned int j)
{
	return (uVelocity(i - 1, j) + uVelocity(i, j)) / 2.0f;
}

static float vVelocityAtPoint(GridStructureHalo<float>& vVelocity, unsigned int i, unsigned int j)
{
	return vVelocity(i, j - 1) + vVelocity(i, j) / 2.0f;
}

static float cubicInterpolate(std::array<float, 4> axis, float x)
{
	return axis[1] + 0.5f * x * (axis[2] - axis[0] + x * (2.0f * axis[0] - 5.0f * axis[1] + 4.0f * axis[2] - axis[3] + x * (3.0f * (axis[1] - axis[2]) + axis[3] - axis[0])));
}

static float bicubicInterpolate(GridStructureHalo<float>& grid, unsigned int i, unsigned int j, float cellWidth)
{
	float x = i * cellWidth;
	float y = j * cellWidth;
	std::array<float, 4> row;
	row[0] = cubicInterpolate({ grid(i - 1, j - 1), grid(i, j - 1), grid(i + 1, j - 1), grid(i + 2, j - 1) }, y);
	row[1] = cubicInterpolate({ grid(i - 1, j), grid(i, j), grid(i + 1, j), grid(i + 2, j) }, y);
	row[2] = cubicInterpolate({ grid(i - 1, j + 1), grid(i, j + 1), grid(i + 1, j + 1), grid(i + 2, j + 1) }, y);
	row[3] = cubicInterpolate({ grid(i - 1, j + 2), grid(i, j + 2), grid(i + 1, j + 2), grid(i + 2, j + 2) }, y);
	return cubicInterpolate(row, x);
}

void Grid2D::advect(float timeStep)
{
	// RK-2
	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{

			float xMid = i * cellWidth - timeStep * uVelocityAtPoint(uVelocity, i, j);
			float yMid = j * cellWidth - timeStep * vVelocityAtPoint(vVelocity, i, j);
			unsigned int iMid, jMid;
			if(clamp_to_grid(xMid, yMid, iMid, jMid))
			{
				uVelocity(i, j) = uVelocity(iMid, jMid);
				vVelocity(i, j) = vVelocity(iMid, jMid);
				continue;
			}

			// Interpolate velocity fields at mid point
			float uMid = bicubicInterpolate(uVelocity, iMid, jMid, cellWidth);
			float vMid = bicubicInterpolate(vVelocity, iMid, jMid, cellWidth);

			float xFinal = i * cellWidth - timeStep * uMid;
			float yFinal = j * cellWidth - timeStep * vMid;
			unsigned int iFinal , jFinal;
			if(clamp_to_grid(xMid, yMid, iFinal, jFinal))
			{
				uVelocity(i, j) = uVelocity(iFinal, jFinal);
				vVelocity(i, j) = vVelocity(iFinal, jFinal);
				continue;
			}

			uVelocity(i, j) = bicubicInterpolate(uVelocity, iFinal, jFinal, cellWidth);
			vVelocity(i, j) = bicubicInterpolate(vVelocity, iFinal, jFinal, cellWidth);
			smoke(i, j) = bicubicInterpolate(smoke, iFinal, jFinal, cellWidth);
		}
	}

}