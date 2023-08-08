#pragma once

#include <vector>
#include <array>
#include <memory>

struct Point
{
	std::array<float, 3> coordinates;

	std::shared_ptr<Point> neighbour1, neighbour2;

	Point(float x, float y, float z) {
		coordinates = { x, y, z };
	};
};

class Polygon
{
public:

	// TODO: Check code for accidental copying (const and &) 

	std::vector<Point> pointList;

	void TriangulatePolygon(int sweepCoord);
};