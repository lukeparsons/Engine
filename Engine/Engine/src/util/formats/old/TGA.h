#pragma once
#include <string>

struct TGAImage
{
	std::string data;
	unsigned __int16 width = 0;
	unsigned __int16 height = 0;
	unsigned __int8 numberBitsInPixel = 0;
	unsigned __int8 dataType = 0;

	bool hasAlpha() const
	{
		// TODO: If the pixel depth is 16 bits, the topmost bit is reserved for transparency.
		if(dataType == 2 || dataType == 10)
		{
			return numberBitsInPixel == 32;
		}
	}

};

TGAImage ReadTGAFile(const char* path);