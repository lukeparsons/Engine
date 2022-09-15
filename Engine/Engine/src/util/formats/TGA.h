#pragma once
#include <string>
#include <bitset>

struct TGAImage
{
	std::string data;
	unsigned __int16 width;
	unsigned __int16 height;
};

TGAImage ReadTGAFile(const char* path);