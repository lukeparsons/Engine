#include "Model.h"
#include <algorithm>
#include <iostream>
#include "OBJ.h"
#include "../../FileIO.h"

Model::Model(const char* fileName)
{
	std::string extension = ReadFileExtension(fileName);

	if(extension == "obj")
	{
		ReadOBJFile(fileName, *this);
	} else
	{
		// Error
	}
}

