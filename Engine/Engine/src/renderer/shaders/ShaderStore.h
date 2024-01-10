#pragma once
#include "ShaderProgram.h"
#include <memory>
#include <map>

class ShaderStore
{
private:
	using ShaderFileNames = std::pair<std::string, std::string>;

	std::map<ShaderFileNames, std::shared_ptr<ShaderProgram>> shaders;
public:
	template<typename T>
	std::shared_ptr<T> LoadShader(const std::string vertexFile, const std::string fragmentFile)
	{
		ShaderFileNames shaderFiles = { vertexFile, fragmentFile };
		if(!shaders.contains(shaderFiles))
		{
			shaders.emplace(shaderFiles, std::make_shared<T>(vertexFile, fragmentFile));
		}
		return std::static_pointer_cast<T>(shaders[shaderFiles]);
	}
};

extern ShaderStore g_shaderStore;
