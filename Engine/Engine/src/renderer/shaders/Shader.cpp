#include "Shader.h"
#include "../../util/FileIO.h"

#define VERTEX_EXTENSION "vertex"
#define FRAGMENT_EXTENSION "fragment"

Shader::Shader(const char* path)
{
	std::string shaderCode = ReadFile(path);
	std::string shaderExtension = ReadFileExtension(path);

	GLenum shaderType;

	if (shaderExtension.compare(VERTEX_EXTENSION) == 0)
	{
		shaderType = GL_VERTEX_SHADER;
	} else if (shaderExtension.compare(FRAGMENT_EXTENSION) == 0)
	{
		shaderType = GL_FRAGMENT_SHADER;
	} else
	{
		throw std::invalid_argument(std::string("Invalid extension provided for shader with path ").append(path));
	}

	GLuint idValue = glCreateShader(shaderType);
	id = idValue;

	size_t shaderCodeLen = shaderCode.length();

	GLchar* shaderCodeGL = new GLchar[shaderCodeLen + 1];
	memcpy(shaderCodeGL, shaderCode.c_str(), shaderCodeLen + 1);

	glShaderSource(idValue, 1, &shaderCodeGL, NULL);
	glCompileShader(idValue);

	delete[] shaderCodeGL;

	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(idValue, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(idValue, sizeof(infoLog), NULL, infoLog);
		throw std::runtime_error(std::string("Failed to compile shader at path ") + path + "\n" + infoLog);
	}
}

Shader::~Shader()
{
	glDeleteShader(id);
}