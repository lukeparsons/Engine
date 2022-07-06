#pragma once
#include <glad/glad.h>
#include "Shader.h"

class ShaderProgram
{
private:

	GLuint id;

public:

	ShaderProgram(Shader& vertexShader, Shader& fragmentShader);
	~ShaderProgram();

	inline GLuint GetID() const { return id; }
};

