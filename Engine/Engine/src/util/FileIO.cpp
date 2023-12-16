#include "FileIO.h"
#include <iostream>
#include <cassert>

/**
* \file
* A collection of functions for reading and writing to files 
*/

/**
* Reads a file at a specified path
* @param path Path of file to read
* @throws std::ifstream::failure Exception on failure to open or read file
* @returns The text in the file
*/

// TODO: this needs cleaning up
std::string ReadFile(const std::string path, std::ios_base::openmode openMode)
{
	std::ifstream file;
	std::string buffer;
	try
	{
		file.open(path, openMode);

		file.seekg(0, file.end);
		std::streamoff length = file.tellg();
		file.seekg(0, file.beg);

		assert(length >= 0);

		buffer = std::string(length, 0);

		file.read(&buffer[0], length);
	} catch(const std::ifstream::failure& e)
	{
		std::cout << "Error " << e.what() << std::endl;
		throw std::ifstream::failure(std::string("Error opening file at path ") + path + " error " + e.what());
	} catch(std::exception e)
	{
		std::cout << "Error " << e.what() << std::endl;
	}

	// A file is "bad" (the badbit is set) a error has occured which prevents the stream being read
	// TODO: probably wrong way to do this
	if(file.bad())
	{
		//throw std::ifstream::failure(std::string("Error reading file at path ") + path);
	}

	file.close();

	return buffer;
}

/**
* Reads the extension of a file name
* @param fileName The full name of the file to read the extension of
* @throws std::ifstream::invalid_argument Exception on failure to read extension
* @returns The extension of the file, not including the dot (.)
*/
std::string ReadFileExtension(const std::string fileName)
{
	std::string extension = std::move(std::string(fileName));

	size_t dotPosition = extension.rfind('.');

	if(dotPosition == std::string::npos)
	{
		throw std::invalid_argument(std::string("Failed to find extension for a file with path") + fileName);
	}

	// Erase all the text in the string up to and including the final dot to just leave the extension text
	extension.erase(extension.begin(), extension.begin() + dotPosition + 1);

	// Make sure an empty string is not returned (i.e the file ends with a dot and has no extension e.g "Hello.")
	if(extension.length() == 0)
	{
		throw std::invalid_argument(std::string("Failed to find extension for a file with path") + fileName);
	}

	return extension;
}