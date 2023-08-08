#include "OBJ.h"
#include "../../FileIO.h"
#include "Model.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <charconv>
#include <algorithm>
#include <list>

enum class ReadState
{
	READ, FACE, VERTEX, TEXCOORD, NORMAL
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
		return ReadState::FACE;
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

static Polygon ReadFace(std::istringstream& values, Model& model, std::vector<float>& vertices, std::vector<float>& texCoords)
{

	// TODO: Double slash support

	std::string vertexIndices;

	Polygon polygon;

	unsigned short numberOfVertices = 0;
	while(std::getline(values, vertexIndices, ' '))
	{
		std::istringstream valueStream(vertexIndices);
		std::string individualString;
		int slashPos = 0;
		while(std::getline(valueStream, individualString, '/'))
		{
			unsigned int indexValue = std::stoi(individualString);
			if(slashPos == 0)
			{
				// Vertex
				numberOfVertices++;

				int start = (indexValue - 1) * 3;

				model.vertices.push_back(vertices[start]);
				model.vertices.push_back(vertices[start + 1]);
				model.vertices.push_back(vertices[start + 2]);

				// Create a new Point from index value
				Point point = Point(vertices[(indexValue - 1) * 3], vertices[(indexValue - 1) * 3 + 1], vertices[(indexValue - 1) * 3 + 2]);

				if(!polygon.pointList.empty())
				{
					point.neighbour1 = std::make_shared<Point>(polygon.pointList.back());
					polygon.pointList.back().neighbour2 = std::make_shared<Point>(point);
				}
				polygon.pointList.push_back(point);

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
	}

	if(!polygon.pointList.empty())
	{
		polygon.pointList.front().neighbour1 = std::make_shared<Point>(polygon.pointList.back());
		polygon.pointList.back().neighbour2 = std::make_shared<Point>(polygon.pointList.front());
	}
	
	if(numberOfVertices == 4)
	{
		model.vertexIndices.push_back(model.vertexIndices[model.vertexIndices.size() - 4]);
		model.vertexIndices.push_back(model.vertexIndices[model.vertexIndices.size() - 3]);
	}

	//polygon.TriangulatePolygon(1); // Sweep along y
	//std::cout << "Y sweep complete" << std::endl;
	polygon.TriangulatePolygon(2); // Sweep along z
	//std::cout << "Z sweep complete" << std::endl;
	return polygon;
}

void ReadOBJFile(const char* path, Model& model)
{
	std::ifstream fileStream(path, std::ios_base::in);

	if(!fileStream)
	{
		// Throw error
	}

	ReadState RS = ReadState::READ;

	std::vector<float> vertices;
	std::vector<float> texCoords;
	
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
				ReadVertex(lineStream, vertices);
				break;
			case ReadState::TEXCOORD:
				ReadVertex(lineStream, texCoords);
				break;
			case ReadState::FACE:
				model.AddFace(ReadFace(lineStream, model, vertices, texCoords));
		}

		//lineStream.clear();
	}

	fileStream.close();
}