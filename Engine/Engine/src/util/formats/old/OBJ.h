#pragma once
#include <vector>
#include "Model.h"

/*struct OBJModel
{
	std::vector<float> vertices;
	std::vector<unsigned int> vertexIndices;
	unsigned int highestIndexValue = 0;

	OBJModel() {};
	OBJModel(std::vector<float> vertices, std::vector<unsigned int> vertexElements, unsigned int vertexCount) 
		: vertices(vertices) {};
}; */

void ReadOBJFile(const char* path, Model& model);