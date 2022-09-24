#pragma once
#include <string>
#include <bitset>
#include <memory>

struct TGAImage
{
	std::unique_ptr<char[]> data;
	unsigned __int16 width;
	unsigned __int16 height;
	unsigned __int8 numberBitsInPixel;
	unsigned __int8 dataType;
};

TGAImage ReadTGAFile(const char* path);