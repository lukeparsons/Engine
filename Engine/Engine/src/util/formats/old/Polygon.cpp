#include "Polygon.h"
#include <iostream>
#include <algorithm>

const float pi = atanf(1.0) * 4;

struct PairCoord
{
	float u, v;
};

static float GetDistance(const Point& p1, const Point& p2)
{
	float xDiff = p2.coordinates[0] - p1.coordinates[0];
	float yDiff = p2.coordinates[1] - p1.coordinates[1];
	float zDiff = p2.coordinates[2] - p1.coordinates[2];
	return abs(sqrtf(powf(xDiff, 2) + powf(yDiff, 2) + powf(zDiff, 2)));
}

static void CreateEdge(Point& point1, Point& point2)
{

}

void Polygon::TriangulatePolygon(int sweepCoord)
{

	// Order vertices by y-coordinate (ascending)
	std::sort(pointList.begin(), pointList.end(),
		[&sweepCoord](Point& p1, Point& p2) {
			if(p1.coordinates[sweepCoord] == p2.coordinates[sweepCoord])
			{
				return p1.coordinates[0] < p2.coordinates[0];
			}
			{
				return p1.coordinates[sweepCoord] > p2.coordinates[sweepCoord];
			}
		}
	);

	std::cout << "New face" << std::endl;

	// TODO: Stopointsweeping for each point!

	Point* lastPoint = pointList.data();

	for(std::vector<Point>::iterator it = pointList.begin(); it != pointList.end(); it++)
	{

		// Make monotone

		Point point = *it;
		Point neighbour1 = *point.neighbour1.get();
		Point neighbour2 = *point.neighbour2.get();

		std::cout << "(" << point.coordinates[0] << ", " << point.coordinates[1] << ", " << point.coordinates[2] << ") -> ";
		std::cout << "(" << neighbour1.coordinates[0] << ", " << neighbour1.coordinates[1] << ", " << neighbour1.coordinates[2] << ")" << " -> " << "(" << neighbour2.coordinates[0] << ", " << neighbour2.coordinates[1] << ", " << neighbour2.coordinates[2] << ")" << std::endl;
		//std::cout << "N1 " << point.neighbour1Idx << " N2 " << point.neighbour2Idx << std::endl;

		// Cosine rule to calculate interior angle of point between it's two neighbours (in radians)
		float a = GetDistance(neighbour1, neighbour2); // distance between neighbours
		float b = GetDistance(neighbour1, point); // distance between one neighbour and point
		float c = GetDistance(neighbour2, point); // distance between other neighbour and point
		std::cout << "Distance neighbours " << a << " Distance two " << b << " Distance three " << c << std::endl;
		float interiorAngle = acosf((powf(b, 2) + powf(c, 2) - powf(a, 2)) / (2 * b * c));

		unsigned int intersectionCount = 0;
		for(std::vector<Point>::iterator it2 = pointList.begin(); it2 != pointList.end(); it2++)
		{

			if(it == it2)
			{
				if(intersectionCount % 2 != 0)
				{
					// Odd intersects mean polygon interior angle is on the other side of the triangle made by the point and it's neighbours
					interiorAngle = 2 * pi - interiorAngle;
				}
				// Otherwise, even intersects means polygon interior angle is inside the triangle made by the point and it's neighbours which we've already calculated
				break;
			}

			if((neighbour2.coordinates[sweepCoord] <= point.coordinates[sweepCoord] && point.coordinates[sweepCoord] <= neighbour1.coordinates[sweepCoord]) || 
				(neighbour1.coordinates[sweepCoord] <= point.coordinates[sweepCoord] && point.coordinates[sweepCoord] <= neighbour2.coordinates[sweepCoord]))
			{
				// Intersects sweepointline
				intersectionCount++;
			}
		}

		float pointSweepCoord = point.coordinates[sweepCoord];
		float neighbour1SweepCoord = neighbour1.coordinates[sweepCoord];
		float neighbour2SweepCoord = neighbour2.coordinates[sweepCoord];

		if(interiorAngle < pi)
		{
			if(neighbour1SweepCoord <= pointSweepCoord && neighbour2SweepCoord <= pointSweepCoord)
			{
				/* Vertex is topointpoint
				Both edges go down */
				std::cout << " Topointpoint" << std::endl;
			} else if(neighbour1SweepCoord >= pointSweepCoord && neighbour2SweepCoord >= pointSweepCoord)
			{
				/* Vertex is bottom point
				Both edges go upointand vertex is outside */
				std::cout << " Bottom point" << std::endl;
			}
		} else if(interiorAngle > pi)
		{
			if(neighbour1SweepCoord > pointSweepCoord && neighbour2SweepCoord > pointSweepCoord)
			{
				/* Vertex is merge point
				Both edges go upoint*/
				std::cout << " Merge point" << std::endl;
			} else if(neighbour1SweepCoord > pointSweepCoord && neighbour2SweepCoord > pointSweepCoord)
			{
				/* Vertex is split point
				Both edges go down */
				std::cout << " Split point" << std::endl;
			}
		}

		/* Otherwise, vertex is regular point (one edge goes upointand one goes down) */

		std::cout << "Interior angle: " << interiorAngle << std::endl;
		std::cout << std::endl;

		lastPoint = &point;
	}
	std::cout << std::endl;
}
