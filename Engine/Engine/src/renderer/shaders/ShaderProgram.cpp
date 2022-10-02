#include "ShaderProgram.h"
#include <iostream>

ShaderProgram::ShaderProgram(Shader& vertexShader, Shader& fragmentShader)
{
	id = glCreateProgram();

	GLuint vertexShaderID = vertexShader.GetID();
	GLuint fragmentShaderID = fragmentShader.GetID();

	glAttachShader(id, vertexShaderID);
	glAttachShader(id, fragmentShaderID);

	glLinkProgram(id);

	GLint success;
	GLchar infoLog[512];
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(id, sizeof(infoLog), NULL, infoLog);
		throw std::runtime_error(std::string("Failed to link shader program \n") + infoLog);
	}
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(id);
}
