#include <fstream>
#include <iostream>
#include "FileIO.h"

std::optional<std::string> ReadFile(std::string path)
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
