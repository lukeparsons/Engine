#pragma once
#include <string>
#include <bitset>

struct TGAImage
{
	std::string data;
	unsigned __int16 width;
	unsigned __int16 height;
	unsigned __int8 numberBitsInPixel;
	unsigned __int8 dataType;
};

TGAImage ReadTGAFile(const char* path);