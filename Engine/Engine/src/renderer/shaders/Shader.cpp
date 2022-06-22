#include "Shader.h"
#include "../../util/FileIO.h"
#include <iostream>

#define VERTEX_EXTENSION "vertex"
#define FRAGMENT_EXTENSION "fragment"

Shader::Shader(const char* path)
{
	std::optional<std::string> shaderCode = ReadFile(path);

	if (!shaderCode.has_value())
	{
		std::cout << "Failed to read shader at path " << path << std::endl;
		id = std::nullopt;
		return;
	}

	std::optional<std::string> shaderExtension = ReadFileExtension(path);

	if (!shaderExtension.has_value())
	{
		std::cout << "Failed to read shader extension at path " << path << std::endl;
		id = std::nullopt;
		return;
	}

	GLenum shaderType;

	if (shaderExtension.value().compare(VERTEX_EXTENSION) == 0)
	{
		shaderType = GL_VERTEX_SHADER;
	} else if (shaderExtension.value().compare(FRAGMENT_EXTENSION) == 0)
	{
		shaderType = GL_FRAGMENT_SHADER;
	} else
	{
		std::cout << "Invalid extension for shader at path " << path << std::endl;
		id = std::nullopt;
		return;
	}

	GLuint idValue = glCreateShader(shaderType);
	id = idValue;
	
	const GLchar* shaderCodeStr = shaderCode.value().c_str();
	glShaderSource(idValue, 1, &shaderCodeStr, NULL);
	glCompileShader(idValue);

	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(idValue, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(idValue, sizeof(infoLog), NULL, infoLog);
		std::cout << "Failed to compile shader at path " << path << "\n" << infoLog << std::endl;
	}
}

Shader::~Shader()
{
	if (id.has_value())
	{
		glDeleteShader(id.value());
	}
}