#pragma once
#include "Shader.h"
#include <glad/glad.h>
#include "../../math/Matrix4f.h"

class ShaderProgram
{
protected:
	GLuint id;
public:

	ShaderProgram(Shader&& vertexShader, Shader&& fragmentShader);
	ShaderProgram(const std::string vertexFile, const std::string fragmentFile);
	~ShaderProgram();

	inline GLuint GetID() const { return id; }

	virtual void Configure(const Matrix4f& cameraMatrix, const Matrix4f& modelMatrix) = 0;
	virtual void SetColour(std::array<float, 3> _colour) = 0;
};

