#include "Shader.h"
#include "../../util/FileIO.h"
#include <iostream>

#define VERTEX_EXTENSION "vertex"
#define FRAGMENT_EXTENSION "fragment"

Shader::Shader(const char* path)
{
	Either<std::string, std::string> shaderCode = ReadFile(path);

	if (shaderCode.isLeft() == true)
	{
		std::cout << shaderCode.fromLeft(DEF_STRING) << std::endl;
		std::cout << "Failed to read shader at path " << path << std::endl;
		id = Maybe<GLuint>();
		return;
	}

	Either<std::string, std::string> shaderExtension = ReadFileExtension(path);

	if (shaderExtension.isLeft() == true)
	{
		std::cout << shaderExtension.fromLeft(DEF_STRING) << std::endl;
		std::cout << "Failed to read shader extension at path " << path << std::endl;
		id = Maybe<GLuint>();
		return;
	}

	GLenum shaderType;

	std::string extension = shaderExtension.fromRight(DEF_STRING);

	if (extension.compare(VERTEX_EXTENSION) == 0)
	{
		shaderType = GL_VERTEX_SHADER;
	} else if (extension.compare(FRAGMENT_EXTENSION) == 0)
	{
		shaderType = GL_FRAGMENT_SHADER;
	} else
	{
		std::cout << "Invalid extension for shader at path " << path << std::endl;
		id = Maybe<GLuint>();
		return;
	}

	GLuint idValue = glCreateShader(shaderType);
	id = idValue;

	const GLchar* shaderCodeStr = shaderCode.fromRight(DEF_STRING).c_str();
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
	if (id.isJust())
	{
		glDeleteShader(id.fromJust());
	}
}