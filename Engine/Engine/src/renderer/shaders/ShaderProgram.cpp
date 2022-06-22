#include "ShaderProgram.h"
#include <iostream>

ShaderProgram::ShaderProgram(Shader& vertexShader, Shader& fragmentShader)
{
	id = glCreateProgram();
	
	GLuint vertexShaderID = vertexShader.GetID().value();
	GLuint fragmentShaderID = fragmentShader.GetID().value();

	glAttachShader(id, vertexShaderID);
	glAttachShader(id, fragmentShaderID);

	glLinkProgram(id);

	GLint success;
	GLchar infoLog[512];
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(id, sizeof(infoLog), NULL, infoLog);
		std::cout << "Failed to link shader program \n" << infoLog << std::endl;
	}
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(id);
}
