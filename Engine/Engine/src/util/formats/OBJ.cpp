#include "../../types/Either.h"
#include "OBJ.h"
#include <string>
#include <sstream>
#include "../FileIO.h"
#include <iostream>
#include <charconv>
#include "../StringUtil.h"

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

static void ReadVertex(std::string values, std::vector<float>& container)
{
	for(std::string valueStr : TokenizeString(values, " "))
	{
		float valueAsFloat;
		std::from_chars_result result = std::from_chars(valueStr.data(), valueStr.data() + valueStr.size(), valueAsFloat);
		// TODO: Check for conversion errors
		container.push_back(valueAsFloat);
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

static void ReadElement(std::string values, OBJModel& model, const std::vector<float>& verticesPos, const std::vector<float>& texCoords)
{

	GeometryType geomType;

	int spaceCount = std::count(values.begin(), values.end(), ' ');

	geomType = static_cast<GeometryType>(spaceCount + 1);

	std::vector<std::string> spacesList = TokenizeString(values, " ");

	if(spacesList.empty())
	{
		return;
	}

	bool isDoubleSlash = (spacesList[0].find("//") != std::string::npos);

	for(int i = 0; i < spacesList.size(); i++)
	{

		std::vector<std::string> values = TokenizeString(spacesList[i], "/");

		for(int x = 0; x < values.size(); x++)
		{
			unsigned int value = std::stoi(values[x]);
			if(x == 0)
			{
				// Vertex
				int start = (value - 1) * 3;
				model.vertices.push_back(verticesPos[start]);
				model.vertices.push_back(verticesPos[start + 1]);
				model.vertices.push_back(verticesPos[start + 2]);
			} else if(x == 1)

			{
				if(isDoubleSlash)
				{
					// Vector Normal
				} else
				{
					// Tex Coord
					int start = (value - 1) * 2;
					model.vertices.push_back(texCoords[start]);
					model.vertices.push_back(texCoords[start + 1]);
					model.vertexIndices.push_back(model.highestIndexValue++);
				}
			} else if(x == 2)
			{
				// Vector normal
			}
		}
	}
	AddToIndex(geomType, model.vertexIndices);
}

OBJModel ReadOBJFile(const char* path)
{

	Either<std::string, std::string> eitherFileText = ReadFile(path);

	// TODO: file error handling

	std::string fileText = eitherFileText.fromRight(DEF_STRING);

	OBJModel model;

	std::vector<float> verticesPos;
	std::vector<float> texCoords;

	ReadState RS = ReadState::READ;

	/* TokenizeString will return an empty vector on failure
	*  If a obj file containing invalid lines or no lines is opened, don't warn/throw error
	*  It's the user's responsibility to provide a valid obj file, it's unlikely they won't as almost certainly exported from 3d modelling software */

	for(std::string line : TokenizeString(fileText, "\n"))
	{
		if(line.empty())
		{
			continue;
		}

		size_t spacePos = line.find(" ");
		if(spacePos == std::string::npos)
		{
			continue;
		}

		std::string firstWord = line.substr(0, spacePos);

		RS = CheckReadState(firstWord);

		line.erase(0, spacePos + 1);

		switch(RS)
		{
			case ReadState::VERTEX:
				ReadVertex(line, verticesPos);
				break;
			case ReadState::TEXCOORD:
				ReadVertex(line, texCoords);
				break;
			case ReadState::ELEMENT:
				ReadElement(line, model, verticesPos, texCoords);
				break;
		}
	}

	return model;
}