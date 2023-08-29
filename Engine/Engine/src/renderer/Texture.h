#pragma once
#include <vector>
#include "../util/formats/png/lodepng.h"

struct Texture
{
	std::vector<unsigned char> data;
	unsigned int width, height;

	Texture(const char* fileName)
	{
		unsigned int error = lodepng::decode(data, width, height, fileName);

		if(error)
		{
			std::cout << "Error occured in loading image " << fileName << ", " << lodepng_error_text(error) << std::endl;
		}
	}
};