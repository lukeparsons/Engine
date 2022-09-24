#include "TGA.h"
#include <fstream>
#include <ios>
#include <memory>

// TODO: this is not particularly good code...
TGAImage ReadTGAFile(const char* path)
{
	std::ifstream file(path, std::ios::out | std::ios::binary);

	file.seekg(0, file.end);
	size_t size = file.tellg();
	file.seekg(0, file.beg);

	TGAImage image;
	image.data = std::make_unique<char[]>(size + 1);
	char* imageData = image.data.get();

	file.read(imageData, size);
	imageData[size] = '\0';

	std::bitset<8>w1{(unsigned long long)imageData[13]};
	std::bitset<8>w2{(unsigned long long)imageData[12]};
	std::bitset<16>widthB{w1.to_string() + w2.to_string()};

	std::bitset<8>h1{(unsigned long long)imageData[15]};
	std::bitset<8>h2{(unsigned long long)imageData[14]};
	std::bitset<16>heightB{h1.to_string() + h2.to_string()};

	image.width = widthB.to_ullong();
	image.height = heightB.to_ullong();
	image.numberBitsInPixel = (int)imageData[16];
	image.dataType = (int)imageData[2];

	imageData = imageData + 18;

	file.close();

	return image;
}