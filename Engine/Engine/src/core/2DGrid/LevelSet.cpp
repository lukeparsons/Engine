#include "Grid2D.h"
#include <limits>
#include "../../types/Maybe.h"

template<typename T>
struct Point
{
	T x, y;
	Point<T>(T _x, T _y) : x(_x), y(_y) {};
};

using Pointf = Point<float>;
using GridPoint = Point<unsigned int>;

// TODO: Make this one thick halo only!
static GridStructure2DHalo<Maybe<Pointf>> closestPoint = GridStructure2DHalo<Maybe<Pointf>>(Nothing(), 0, 0);

static float calculateDistance(float x1, float y1, float x2, float y2)
{
	return sqrtf(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

static void propagate_point(unsigned int i, unsigned int j, unsigned int iNeighbour, unsigned int jNeighbour, unsigned int column, GridStructure2DHalo<float>& signedDistance)
{
	if(closestPoint(iNeighbour, jNeighbour).isJust())
	{
		Pointf p = closestPoint(iNeighbour, jNeighbour).fromJust();
		float d = calculateDistance(p.x, p.y, i, j);
		if(d < signedDistance(i, j))
		{
			signedDistance(i, j) = d;
			closestPoint(i, j) = p;
		}
	}
}

void Grid2D::calculate_initial_distances()
{
	closestPoint = GridStructure2DHalo<Maybe<Pointf>>(Nothing(), column, row);

	for(unsigned int i = 0; i < column; i++) {
		for(unsigned int j = 0; j < row; j++)
		{
			if(gridData(i, j).cellState == GridDataPoint2D::SOLID)
			{
				signedDistance(i, j) = 0;
				closestPoint(i, j) = Pointf(i, j);
			} else
			{
				signedDistance(i, j) = std::numeric_limits<float>::max();
			}
		}
	}

	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			propagate_point(i, j, i + 1, j, column, signedDistance);
			propagate_point(i, j, i - 1, j, column, signedDistance);
			propagate_point(i, j, i, j + 1, column, signedDistance);
			propagate_point(i, j, i, j - 1, column, signedDistance);
		}
	}
}

void Grid2D::extrapolate()
{
	
	std::vector<GridPoint> wavefront;

	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			if(signedDistance(i, j) != 0)
			{
				if(signedDistance(i + 1, j) == 0 || signedDistance(i - 1, j) == 0
					|| signedDistance(i, j + 1) == 0 || signedDistance(i, j - 1) == 0)
				{
					signedDistance(i, j) = 1;
					wavefront.push_back(Point(i, j));
				}
			}
		}
	}
	for(size_t t = 0; t < wavefront.size(); t++)
	{
		unsigned int i = wavefront[t].x;
		unsigned int j = wavefront[t].y;

		float d = signedDistance(i, j);
		float uAvg = 0;
		float vAvg = 0;
		int count = 0;
		if(signedDistance(i + 1, j) < d)
		{
			uAvg += uVelocity(i + 1, j);
			vAvg += vVelocity(i + 1, j);
			count++;
		} else if(signedDistance(i + 1, j) == std::numeric_limits<float>::max())
		{
			signedDistance(i + 1, j) = d + 1;
			wavefront.push_back(GridPoint(i + 1, j));
		}
		if(signedDistance(i - 1, j) < d)
		{
			uAvg += uVelocity(i - 1, j);
			vAvg += vVelocity(i - 1, j);
			count++;
		} else if(signedDistance(i - 1, j) == std::numeric_limits<float>::max())
		{
			signedDistance(i - 1, j) = d + 1;
			wavefront.push_back(GridPoint(i - 1, j));
		}
		if(signedDistance(i, j + 1) < d)
		{
			uAvg += uVelocity(i, j + 1);
			vAvg += vVelocity(i, j + 1);
			count++;
		} else if(signedDistance(i, j + 1) == std::numeric_limits<float>::max())
		{
			signedDistance(i, j + 1) = d + 1;
			wavefront.push_back(GridPoint(i, j + 1));
		}
		if(signedDistance(i, j - 1) < d)
		{
			uAvg += uVelocity(i, j - 1);
			vAvg += vVelocity(i, j - 1);
			count++;
		} else if(signedDistance(i, j - 1) == std::numeric_limits<float>::max())
		{
			signedDistance(i, j - 1) = d + 1;
			wavefront.push_back(GridPoint(i, j - 1));
		}

		if(count > 0)
		{
			uVelocity(i, j) = uAvg / count;
			vVelocity(i, j) = vAvg / count;
		}
	}
}