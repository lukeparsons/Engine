#include <fstream>
#include <iostream>
#include "FileIO.h"

Either<std::string, std::string> ReadFile(const char* path)
{
	std::fstream file(path);

	if (!file.is_open())
	{
		return Left<std::string, std::string>(std::string("Failed to open file at path").append(path));
	}

	std::string buffer;

	std::getline(file, buffer, '\0');

	file.close();

	return Right<std::string, std::string>(buffer);
}

Either<std::string, std::string> ReadFileExtension(const char* path)
{
	std::string pathStr = std::string(path);

	size_t dotPosition = pathStr.rfind('.');

	if (dotPosition == std::string::npos)
	{
		return Left<std::string, std::string>(std::string("Failed to find extension for a file with path ").append(path));
	}

	// Erase all the text in the string up to and including the final dot to just leave the extension text
	pathStr.erase(pathStr.begin(), pathStr.begin() + dotPosition + 1);

	// Make sure an empty string is not returned
	if (pathStr.length() == 0)
	{
		return Left<std::string, std::string>(std::string("Failed to find extension for a file with path ").append(path));
	}

	return Right<std::string, std::string>(pathStr);
}