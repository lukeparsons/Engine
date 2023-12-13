#include "Grid2D.h"
#include <limits>
#include "../../types/Maybe.h"

static struct Point
{
	float x, y;
	Point(float _x, float _y) : x(_x), y(_y) {};
};

// TODO: Make this one thick halo only!
static GridStructureHalo<Maybe<Point>> closestPoint = GridStructureHalo<Maybe<Point>>(Nothing(), 0, 0);

static float calculateDistance(float x1, float y1, float x2, float y2)
{
	return sqrtf(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

static void propagate_point(unsigned int i, unsigned int j, unsigned int iNeighbour, unsigned int jNeighbour, unsigned int column, GridStructureHalo<float>& signedDistance)
{
	if(closestPoint(iNeighbour, jNeighbour).isJust())
	{
		Point p = closestPoint(iNeighbour, jNeighbour).fromJust();
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
	closestPoint = GridStructureHalo<Maybe<Point>>(Nothing(), column, row);

	for(unsigned int i = 0; i < column; i++) {
		for(unsigned int j = 0; j < row; j++)
		{
			if(gridData(i, j).cellState == GridDataPoint::SOLID)
			{
				signedDistance(i, j) = 0;
				closestPoint(i, j) = Point(i, j);
				std::cout << closestPoint(i, j).isJust() << std::endl;
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

	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			if(gridData(i, j).cellState == GridDataPoint::SOLID)
			{
				std::cout << "Solid (" << i << ", " << j << ") " << signedDistance(i, j) << std::endl;
			} else
			{
				std::cout << "Fluid (" << i << ", " << j << ") " << signedDistance(i, j) << std::endl;
			}
		}
	}

}