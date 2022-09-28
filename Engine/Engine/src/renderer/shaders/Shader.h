#pragma once
#include <glad/glad.h>
#include "../../types/Maybe.h"

class Shader
{
private:

	GLuint id;

public:
	Shader(const char* path);
	~Shader();

	inline GLuint GetID() const { return id; }
};
