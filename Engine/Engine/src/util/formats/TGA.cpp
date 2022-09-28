#include "TGA.h"
#include "../FileIO.h"
#include <ios>
#include <bitset>

static std::bitset<16> ReadTwoBytes(char firstByte, char secondByte)
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
	image.width = static_cast<unsigned short>(widthBytes.to_ulong());
	image.height = static_cast<unsigned short>(heightBytes.to_ulong());
	image.numberBitsInPixel = static_cast<int>(tgaData[16]);
	image.dataType = static_cast<int>(tgaData[2]);

	tgaData.erase(0, 17);

	return image;
}