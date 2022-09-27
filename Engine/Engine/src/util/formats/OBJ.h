#pragma once
#include <vector>

struct OBJModel
{
	std::vector<float> vertices;
	std::vector<unsigned int> vertexIndices;
	unsigned int highestIndexValue = 0;

	OBJModel() {};
	OBJModel(std::vector<float> vertices, std::vector<unsigned int> vertexElements, unsigned int vertexCount) 
		: vertices(vertices) {};
};

OBJModel ReadOBJFile(const char* path);