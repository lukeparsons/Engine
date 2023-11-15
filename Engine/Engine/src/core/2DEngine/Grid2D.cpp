#include "Grid2D.h"
#include <algorithm>

static std::array<float, 4> get_interp_weights(float s)
{
	float sSquared = powf(s, 2);
	float sCubed = powf(s, 3);

	float negativeWeight = (-1 / 3.0f) * s + (1 / 2.0f) * sSquared - (1 / 6.0f) * sCubed;
	float weight = 1 - sSquared + (1 / 2.0f) * (sCubed - s);
	float positiveWeight = s + (1 / 2.0f) * (sSquared - sCubed);
	float doublePositiveWeight = (1 / 6.0f) * (sCubed - s);

	return { negativeWeight, weight, positiveWeight, doublePositiveWeight };
}

static float interpolate(const GridStructureHalo<float>& field, float x, float y, unsigned int i, unsigned int j, float cellWidth)
{
	float w01 = x;
	float w11 = y;
	float w00 = 1 - w01;
	float w10 = 1 - w11;

	return w00* w10* field(i, j) + w01 * w10 * field(i + 1, j) + w00 * w11 * field(i, j + 1) + w01 * w11 * field(i + 1, j + 1);
}

static void clamp(unsigned int& originali, unsigned int& originalj, float& x, float& y, float scale, float cellWidth, unsigned int column, unsigned int row)
{
	int i = static_cast<int>(std::roundf(x * scale)); // gives rounded x coord on grid
	int j = static_cast<int>(std::roundf(y * scale)); // gives rounded y coord on grid

	// Clamp to edges
	originali = std::min(static_cast<unsigned int>(std::max(i, 0)), column);
	originalj = std::min(static_cast<unsigned int>(std::max(j, 0)), row);

	x = std::min(std::max(x, 0.0f), cellWidth * column);
	y = std::min(std::max(y, 0.0f), cellWidth * row);
}

static float getAverageVvelocity(const GridStructureHalo<float>& vVelocity, unsigned int i, unsigned int j)
{
	return (vVelocity(i, j) + vVelocity(i, j + 1) + vVelocity(i - 1, j) + vVelocity(i - 1, j + 1)) * 0.25;
}

static float getAverageUvelocity(const GridStructureHalo<float>& uVelocity, unsigned int i, unsigned int j)
{
	return (uVelocity(i, j) + uVelocity(i, j - 1) + uVelocity(i + 1, j) + uVelocity(i + 1, j - 1)) * 0.25;
}


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
				// u-component
				float x = i * cellWidth - timeStep * uVelocity(i, j); // gives 'true' x coord
				float trueX = x;
				float y = j * cellWidth - timeStep * getAverageVvelocity(vVelocity, i, j); // gives 'true' y coord

				unsigned int originali, originalj;
				clamp(originali, originalj, x, y, scale, cellWidth, column, row);
				uVelocity(i, j) = interpolate(uVelocity, x, y, originali, originalj, cellWidth);

				//std::cout << "From (" << i << ", " << j << ") to (" << originali << ", " << originalj << ")" << std::endl;
				
				// v-component
				x = i * cellWidth - timeStep * getAverageUvelocity(uVelocity, i, j); // gives 'true' x coord
				y = j * cellWidth - timeStep * vVelocity(i, j); // gives 'true' y coord
				float trueY = y;

				clamp(originali, originalj, x, y, scale, cellWidth, column, row);
				vVelocity(i, j) = interpolate(vVelocity, x, y, originali, originalj, cellWidth);

				// smoke
				clamp(originali, originalj, trueX, trueY, scale, cellWidth, column, row);
				smoke(i, j) = interpolate(smoke, trueX, trueY, originali, originalj, cellWidth);
				if(smoke(i, j) < 0)
				{
					std::cout << smoke(i, j) << std::endl;
				}

			}
		}
	}
	//std::cout << "Number of same: " << count << std::endl;
}