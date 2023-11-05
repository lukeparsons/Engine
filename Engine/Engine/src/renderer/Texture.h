#pragma once
#include <vector>
#include "../util/formats/png/lodepng.h"

struct Texture
{
	std::vector<float> pixels;
	unsigned int width, height;
	GLenum format;
	GLenum type;

	Texture(const char* fileName) : format(GL_RGBA), type(GL_UNSIGNED_BYTE)
	{
		//unsigned int error = lodepng::decode(pixels, width, height, fileName);
		unsigned int error = 0;
		if(error)
		{
			std::cout << "Error occured in loading image " << fileName << ", " << lodepng_error_text(error) << std::endl;
		}
	}

	Texture(unsigned int _width, unsigned int _height, const std::vector<float>& _pixels, GLenum _format, GLenum _type) : 
		width(_width), height(_height), pixels(_pixels), format(_format), type(_type) {};
};