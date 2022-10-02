#pragma once
#include "Shader.h"
#include <glad/glad.h>

class ShaderProgram
{
private:
	GLuint id;

public:

	ShaderProgram(Shader& vertexShader, Shader& fragmentShader);
	~ShaderProgram();

	inline GLuint GetID() const { return id; }
};

