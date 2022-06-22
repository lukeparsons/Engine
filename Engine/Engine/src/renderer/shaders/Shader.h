#pragma once
#include <glad/glad.h>
#include <optional>

class Shader
{
public:
	std::optional<GLuint> id;

	Shader(const char* path);
	~Shader();
};
