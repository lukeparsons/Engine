#include <fstream>
#include <iostream>
#include "FileIO.h"

std::optional<std::string> ReadFile(const char* path)
{
	std::fstream file(path);

	if (!file.is_open())
	{
		std::cout << "Failed to open file at path " << path << std::endl;
		return std::nullopt;
	}

	std::string buffer;

	std::getline(file, buffer, '\0');

	file.close();

	return std::optional(buffer);
}

std::optional<std::string> ReadFileExtension(const char* path)
{

	std::string pathStr = std::string(path);

	size_t dotPosition = pathStr.rfind('.');

	if (dotPosition == std::string::npos)
	{
		std::cout << "Failed to find extension for a file with path " << path << std::endl;
		return std::nullopt;
	}

	// Erase all the text in the string up to and including the final dot to just leave the extension text
	pathStr.erase(pathStr.begin(), pathStr.begin() + dotPosition + 1);

	// Make sure an empty string is not returned
	if (pathStr.length() == 0)
	{
		std::cout << "Failed to find extension for a file with path " << path << std::endl;
		return std::nullopt;
	}

	return std::optional(pathStr);
}
