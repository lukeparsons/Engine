#include "Grid3D.h"
#include <limits>
#include "../../types/Maybe.h"

static float calculateDistance(unsigned int x1, unsigned int y1, unsigned int z1, unsigned int x2, unsigned int y2, unsigned int z2)
{
	return sqrtf(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2));
}

static void propagate_point(unsigned int i, unsigned int j, unsigned int k, unsigned int iNeighbour, unsigned int jNeighbour, unsigned int zNeighbour, unsigned int column, GridStructureHalo<float>& signedDistance, GridStructureHalo<Maybe<GridPoint>>& closestPoint)
{
	if(closestPoint(iNeighbour, jNeighbour, zNeighbour).isJust())
	{
		GridPoint p = closestPoint(iNeighbour, jNeighbour, zNeighbour).fromJust();
		float d = calculateDistance(p.x, p.y, p.z, i, j, k);
		if(d < signedDistance(i, j, k))
		{
			signedDistance(i, j, k) = d;
			closestPoint(i, j, k) = p;
		}
	}
}

void Grid3D::calculate_initial_distances()
{ 

	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				if(gridData(i, j, k).cellState == GridDataPoint::SOLID)
				{
					signedDistance(i, j, k) = 0.0f;
					closestPoint(i, j, k) = GridPoint(i, j, k);
				}
			}
		}
	}

	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				propagate_point(i, j, k, i + 1, j, k, column, signedDistance, closestPoint);
				propagate_point(i, j, k, i - 1, j, k, column, signedDistance, closestPoint);
				propagate_point(i, j, k, i, j + 1, k, column, signedDistance, closestPoint);
				propagate_point(i, j, k, i, j - 1, k, column, signedDistance, closestPoint);
				propagate_point(i, j, k, i, j, k + 1, column, signedDistance, closestPoint);
				propagate_point(i, j, k, i, j, k - 1, column, signedDistance, closestPoint);
			}
		}
	}
}

void Grid3D::extrapolate() {

	std::vector<GridPoint> wavefront;

	unsigned int count = 0;
	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				if(signedDistance(i, j, k) != 0)
				{
					if(signedDistance(i + 1, j, k) == 0 || signedDistance(i - 1, j, k) == 0
						|| signedDistance(i, j + 1, k) == 0 || signedDistance(i, j - 1, k) == 0
						|| signedDistance(i, j, k + 1) == 0 || signedDistance(i, j, k - 1) == 0)
					{
						signedDistance(i, j, k) = 1;
						wavefront.push_back(GridPoint(i, j, k));
					}
				}
			}
		}
	}
	//std::cout << wavefront[1079].z << std::endl;

	for(size_t t = 0; t < wavefront.size(); ++t)
	{

		unsigned int i = wavefront[t].x;
		unsigned int j = wavefront[t].y;
		unsigned int k = wavefront[t].z;

		float d = signedDistance(i, j, k);
		float uAvg = 0;
		float vAvg = 0;
		float wAvg = 0;
		int count = 0;
		if(signedDistance(i + 1, j, k) < d)
		{
			uAvg += uVelocity.get(i + 1, j, k);
			vAvg += vVelocity.get(i + 1, j, k);
			wAvg += wVelocity.get(i + 1, j, k);
			count++;
		} else if(signedDistance(i + 1, j, k) == std::numeric_limits<float>::max())
		{
			signedDistance(i + 1, j, k) = d + 1;
			if(i < column - 1)
			{
				wavefront.push_back(GridPoint(i + 1, j, k));
			}
		}
		if(signedDistance(i - 1, j, k) < d)
		{
			uAvg += uVelocity.get(i - 1, j, k);
			vAvg += vVelocity.get(i - 1, j, k);
			wAvg += wVelocity.get(i - 1, j, k);
			count++;
		} else if(signedDistance(i - 1, j, k) == std::numeric_limits<float>::max())
		{
			signedDistance(i - 1, j, k) = d + 1;
			if(i > 0)
			{
				wavefront.push_back(GridPoint(i - 1, j, k));
			}
		}
		if(signedDistance(i, j + 1, k) < d)
		{
			uAvg += uVelocity.get(i, j + 1, k);
			vAvg += vVelocity.get(i, j + 1, k);
			wAvg += wVelocity.get(i, j + 1, k);
			count++;
		} else if(signedDistance(i, j + 1, k) == std::numeric_limits<float>::max())
		{
			signedDistance(i, j + 1, k) = d + 1;
			if(j < row - 1)
			{
				wavefront.push_back(GridPoint(i, j + 1, k));
			}
		}
		if(signedDistance(i, j - 1, k) < d)
		{
			uAvg += uVelocity.get(i, j - 1, k);
			vAvg += vVelocity.get(i, j - 1, k);
			wAvg += wVelocity.get(i, j - 1, k);
			count++;
		} else if(signedDistance(i, j - 1, k) == std::numeric_limits<float>::max())
		{
			signedDistance(i, j - 1, k) = d + 1;
			if(j > 0)
			{
				wavefront.push_back(GridPoint(i, j - 1, k));
			}
		}
		if(signedDistance(i, j, k + 1) < d)
		{
			uAvg += uVelocity.get(i, j, k + 1);
			vAvg += vVelocity.get(i, j, k + 1);
			wAvg += wVelocity.get(i, j, k + 1);
			count++;
		} else if(signedDistance(i, j, k + 1) == std::numeric_limits<float>::max())
		{
			signedDistance(i, j, k + 1) = d + 1;
			if(k < depth - 1)
			{
				wavefront.push_back(GridPoint(i, j, k + 1));
			}
		}
		if(signedDistance(i, j, k - 1) < d)
		{
			uAvg += uVelocity.get(i, j, k - 1);
			vAvg += vVelocity.get(i, j, k - 1);
			wAvg += wVelocity.get(i, j, k - 1);
			count++;
		} else if(signedDistance(i, j, k - 1) == std::numeric_limits<float>::max())
		{
			signedDistance(i, j, k - 1) = d + 1;
			if(k > 0)
			{
				wavefront.push_back(GridPoint(i, j, k - 1));
			}
		}

		if(count > 0)
		{
			uVelocity(i, j, k) = uAvg / count;
			vVelocity(i, j, k) = vAvg / count;
			wVelocity(i, j, k) = wAvg / count;
		}
	}
}