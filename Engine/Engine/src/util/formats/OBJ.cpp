#include "OBJ.h"
#include "../FileIO.h"
#include "../../types/Either.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <charconv>

enum class ReadState
{
	READ, ELEMENT, VERTEX, TEXCOORD, NORMAL
};

static ReadState CheckReadState(std::string firstWord)
{
	if(firstWord == "v")
	{
		return ReadState::VERTEX;
	} else if(firstWord == "vt")
	{
		return ReadState::TEXCOORD;
	} else if(firstWord == "f")
	{
		return ReadState::ELEMENT;
	}

	return ReadState::READ;
}

static void ReadVertex(std::istringstream& line, std::vector<float>& container)
{
	float value;
	while(line >> value)
	{
		container.push_back(value);
	}
}

enum class GeometryType
{
	TRIANGLE = 3, QUAD = 4
};

static void AddToIndex(GeometryType geomType, std::vector<unsigned int>& index)
{
	if(geomType == GeometryType::TRIANGLE)
	{
		return;
	} else if(geomType == GeometryType::QUAD)
	{
		index.push_back(index[index.size() - 4]);
		index.push_back(index[index.size() - 3]);
	}
}

static void ReadElement(std::istringstream& values, OBJModel& model, const std::vector<float>& verticesPos, const std::vector<float>& texCoords)
{

	// TODO: Double slash support

	std::string vertexIndices;
	unsigned short numberOfVerticesInFace = 0;
	while(std::getline(values, vertexIndices, ' '))
	{
		std::istringstream valueStream(vertexIndices);
		std::string individualValue;
		int slashPos = 0;
		while(std::getline(valueStream, individualValue, '/'))
		{
			unsigned int indexValue = std::stoi(individualValue);
			if(slashPos == 0)
			{
				// Vertex
				int start = (indexValue - 1) * 3;
				model.vertices.push_back(verticesPos[start]);
				model.vertices.push_back(verticesPos[start + 1]);
				model.vertices.push_back(verticesPos[start + 2]);
			} else if(slashPos == 1)
			{
				// Tex Coord
				int start = (indexValue - 1) * 2;
				model.vertices.push_back(texCoords[start]);
				model.vertices.push_back(texCoords[start + 1]);
				model.vertexIndices.push_back(model.highestIndexValue++);
			} else if(slashPos == 2)
			{
				// Vector Normal
			}
			slashPos++;
		}
		numberOfVerticesInFace++;
	}
	GeometryType geomType = static_cast<GeometryType>(numberOfVerticesInFace - 1);
	AddToIndex(geomType, model.vertexIndices);
}

OBJModel ReadOBJFile(const char* path)
{

	std::ifstream fileStream(path, std::ios_base::in);

	if(!fileStream)
	{
		// Throw error
	}

	OBJModel model;

	std::vector<float> verticesPos;
	std::vector<float> texCoords;

	ReadState RS = ReadState::READ;
	
	std::string line;
	while(std::getline(fileStream, line))
	{
		if(line.empty())
		{
			continue;
		}

		std::istringstream lineStream(line);

		std::string word;
		lineStream >> word;
		RS = CheckReadState(word);

		switch(RS)
		{
			case ReadState::VERTEX:
				ReadVertex(lineStream, verticesPos);
				break;
			case ReadState::TEXCOORD:
				ReadVertex(lineStream, texCoords);
				break;
			case ReadState::ELEMENT:
				ReadElement(lineStream, model, verticesPos, texCoords);
		}

		//lineStream.clear();
	}

	fileStream.close();
		
	return model;
}