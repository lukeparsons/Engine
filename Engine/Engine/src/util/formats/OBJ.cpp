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

static unsigned int CharAsReadUIntValue(char c)
{
	return static_cast<unsigned int>(c - '0');
}

static ReadState CheckReadState(std::string firstWord)
{
	if(firstWord == "v")
	{
		return ReadState::VERTEX;
	} else if(firstWord == "f")
	{
		return ReadState::ELEMENT;
	}

	return ReadState::READ;
}

static void ReadVertex(std::string values, OBJModel& model)
{
	for(std::string valueStr : TokenizeString(values, ' '))
	{
		float valueAsFloat;
		std::from_chars_result result = std::from_chars(valueStr.data(), valueStr.data() + valueStr.size(), valueAsFloat);
		// TODO: Check for conversion errors
		model.vertices.push_back(valueAsFloat);
	}
}

enum class GeometryType
{
	TRIANGLE = 3, QUAD = 4
};

static void ReadElement(std::string values, OBJModel& model)
{
#define VERTEXELEMENT 0
#define TEXCOORDELEMENT 1
#define NORMALELEMENT 2

	int elementType = VERTEXELEMENT;
	GeometryType geomType;

	int spaceCount = std::count(values.begin(), values.end(), ' ');

	geomType = static_cast<GeometryType>(spaceCount + 1);

	std::vector<std::string> spacesList = TokenizeString(values, ' ');

	for(std::string valueStr : spacesList)
	{
		elementType = VERTEXELEMENT;
		for(int i = 0; i < valueStr.length(); i++)
		{
			char c = valueStr[i];
			if(std::isdigit(c))
			{
				if(elementType == VERTEXELEMENT)
				{
					if(geomType == GeometryType::QUAD)
					{
						if(valueStr == spacesList.back())
						{
							model.vertexIndices.push_back(model.vertexIndices[model.vertexIndices.size() - 1]);
							model.vertexIndices.push_back(CharAsReadUIntValue(c) - 1);
							model.vertexIndices.push_back(model.vertexIndices[model.vertexIndices.size() - 5]);
						} else
						{
							model.vertexIndices.push_back(CharAsReadUIntValue(c) - 1);
						}
					}
					
				} else if(elementType == TEXCOORDELEMENT)
				{
					// Implement
				} else if(elementType == NORMALELEMENT)
				{
					// Implement
				}
			}
			else if(c == '/')
			{
				if(valueStr[i + 1] < valueStr.length() && valueStr[i + 1] == '/')
				{
					elementType = NORMALELEMENT;
				} else
				{
					elementType++;
				}
			}
		}
	}

	
}

OBJModel ReadOBJFile(const char* path)
{
	Either<std::string, std::string> eitherFileText = ReadFile(path);

	// TODO: file error handling

	std::string fileText = eitherFileText.fromRight(DEF_STRING);

	OBJModel model;

	ReadState RS = ReadState::READ;

	/* TokenizeString will return an empty vector on failure
	*  If a obj file containing invalid lines or no lines is opened, don't warn/throw error
	*  It's the user's responsibility to provide a valid obj file, it's unlikely they won't as almost certainly exported from 3d modelling software */

	for(std::string line : TokenizeString(fileText, '\n'))
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
				ReadVertex(line, model);
				break;
			case ReadState::ELEMENT:
				ReadElement(line, model);
				break;
		}
	}

	return model;
}