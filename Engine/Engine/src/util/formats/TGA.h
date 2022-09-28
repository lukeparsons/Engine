#pragma once
#include <string>
#include <bitset>
#include <memory>

struct TGAImage
{
	std::unique_ptr<char[]> data;
	unsigned __int16 width = 0;
	unsigned __int16 height = 0;
	unsigned __int8 numberBitsInPixel = 0;
	unsigned __int8 dataType = 0;
};

TGAImage ReadTGAFile(const char* path);