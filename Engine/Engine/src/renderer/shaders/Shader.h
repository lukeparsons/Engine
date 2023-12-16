#pragma once
#include <glad/glad.h>
#include <string>

class Shader
{
private:
	GLuint id;

public:
	Shader(const std::string path);
	~Shader();

	inline GLuint GetID() const { return id; }
};
