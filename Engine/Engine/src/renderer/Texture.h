#pragma once
#include <vector>
#include <any>
#include "../util/formats/png/lodepng.h"

struct Texture
{
	virtual void* GetPixelData() = 0;
	unsigned int width, height;
	GLenum format;
	GLenum type;

	Texture(unsigned int _width, unsigned int _height, GLenum _format, GLenum _type) :
		width(_width), height(_height), format(_format), type(_type)
	{};
};

template<typename T>
struct TextureData : public Texture
{
	std::vector<T> pixels;

	TextureData(unsigned int _width, unsigned int _height, GLenum _format, GLenum _type, std::vector<T> _pixels) : Texture(_width, _height, _format, _type), pixels(_pixels) {};

	virtual void* GetPixelData() override
	{
		return pixels.data();
	}
};

inline TextureData<unsigned char> LoadPng(const char* fileName)
{
	std::vector<unsigned char> pixels;
	unsigned int width, height;
	unsigned int error = lodepng::decode(pixels, width, height, fileName);
	if(error)
	{
		std::cout << "Error occured in loading image " << fileName << ", " << lodepng_error_text(error) << std::endl;
	}
	return TextureData<unsigned char>(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}
