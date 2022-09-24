#pragma once
#include <vector>

struct OBJModel
{
	std::vector<float> vertices;
	std::vector<unsigned int> vertexIndices;

	OBJModel() {};
	OBJModel(std::vector<float> vertices, std::vector<unsigned int> vertexElements) : vertices(vertices), vertexIndices(vertexIndices) {};
};

OBJModel ReadOBJFile(const char* path);