#include "ShaderProgram.h"
#include <stdexcept>

ShaderProgram::ShaderProgram(Shader&& vertexShader, Shader&& fragmentShader)
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
	std::cout << "Compiled shader program with ID " << id << std::endl;
}

ShaderProgram::ShaderProgram(const std::string vertexFile, const std::string fragmentFile) : ShaderProgram(Shader(vertexFile), Shader(fragmentFile)) {};

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(id);
	std::cout << "Deleted " << id << std::endl;
}