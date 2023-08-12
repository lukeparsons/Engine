#include "TGA.h"
#include "../../FileIO.h"
#include <ios>
#include <bitset>
#include <iostream>

static std::bitset<16> ReadTwoBytes(const char firstByte, const char secondByte)
{
	std::bitset<8>w1{ static_cast<unsigned __int64>(firstByte) };
	std::bitset<8>w2{ static_cast<unsigned __int64>(secondByte) };
	return std::bitset<16>{ w1.to_string() + w2.to_string() };
}

TGAImage ReadTGAFile(const char* path)
{
	std::string tgaData = ReadFile(path, std::ios::binary);

	std::bitset<16> widthBytes = ReadTwoBytes(tgaData[13], tgaData[12]);
	std::bitset<16> heightBytes = ReadTwoBytes(tgaData[15], tgaData[14]);

	TGAImage image;
	image.data = tgaData;
	image.width = static_cast<__int16>(widthBytes.to_ulong());
	image.height = static_cast<__int16>(heightBytes.to_ulong());
	image.numberBitsInPixel = static_cast<__int8>(tgaData[16]);
	image.dataType = static_cast<__int8>(tgaData[2]);

	tgaData.erase(0, 17);

	return image;
}