#include "FileIO.h"
#include <fstream>

/**
* Reads a file
* @param path Path of file to read
*/
std::string ReadFile(const char* path)
{
	try
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		file.open(path, std::ios::in);

		std::string buffer;

		std::getline(file, buffer, '\0');

		file.close();

		return buffer;
	} catch(std::ifstream::failure e)
	{
		throw e.what();
	}
}

std::string ReadFileExtension(const char* path)
{
	std::string pathStr = std::move(std::string(path));

	size_t dotPosition = pathStr.rfind('.');

	if(dotPosition == std::string::npos)
	{
		throw std::invalid_argument(std::string("Failed to find extension for a file with path").append(path));
	}

	// Erase all the text in the string up to and including the final dot to just leave the extension text
	pathStr.erase(pathStr.begin(), pathStr.begin() + dotPosition + 1);

	// Make sure an empty string is not returned
	if(pathStr.length() == 0)
	{
		throw std::invalid_argument(std::string("Failed to find extension for a file with path").append(path));
	}

	return pathStr;
}