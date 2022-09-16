#include "TGA.h"
#include <fstream>
#include <ios>

TGAImage ReadTGAFile(const char* path)
{
	std::ifstream file(path, std::ios::out | std::ios::binary);

	file.seekg(0, file.end);
	int size = file.tellg();
	file.seekg(0, file.beg);

	char* buffer = new char[size + 1];

	file.read(buffer, size);
	buffer[size] = '\0';

	TGAImage image;
	image.data.assign(buffer, size);

	std::bitset<8>w1{(unsigned long long)image.data[13]};
	std::bitset<8>w2{(unsigned long long)image.data[12]};
	std::bitset<16>widthB{w1.to_string() + w2.to_string()};

	std::bitset<8>h1{(unsigned long long)image.data[15]};
	std::bitset<8>h2{(unsigned long long)image.data[14]};
	std::bitset<16>heightB{h1.to_string() + h2.to_string()};

	image.width = widthB.to_ullong();
	image.height = heightB.to_ullong();
	image.numberBitsInPixel = (int)image.data[16];
	image.dataType = (int)image.data[2];

	image.data.erase(18);

	file.close();

	delete[] buffer;

	return image;
}