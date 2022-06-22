#pragma once
#include <glad/glad.h>
#include <optional>

class Shader
{
private:
	std::optional<GLuint> id;
public:

	Shader(const char* path);
	~Shader();

	inline const std::optional<GLuint> GetID() { return id; }
};
