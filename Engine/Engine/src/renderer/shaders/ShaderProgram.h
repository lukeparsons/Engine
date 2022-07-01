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

	inline const GLuint GetID() { return id; }
};

