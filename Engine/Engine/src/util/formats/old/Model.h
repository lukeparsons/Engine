#pragma once

#include "Polygon.h"
#include <vector>
#include <list>

class Model
{
private:
	std::vector<Polygon> faces;
public:

	Model(const char* filename);

	unsigned int highestIndexValue = 0;

	// Temp
	std::vector<unsigned int> vertexIndices;

	std::vector<float> vertices;

	void AddFace(const Polygon& polygon)
	{
		faces.push_back(polygon);
	}

	/*Vertex GetVertex(unsigned int index) const
	{
		return Vertex(vertices[index], vertices[index + 1], vertices[index + 2]);
	}*/
};