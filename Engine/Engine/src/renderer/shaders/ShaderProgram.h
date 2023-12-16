#pragma once
#include "Shader.h"
#include <glad/glad.h>
#include "../../math/Matrix4f.h"

class ShaderProgram
{
private:
	GLuint id;
protected:
	unsigned int transformLoc;
	unsigned int modelLoc;
public:

	ShaderProgram(Shader&& vertexShader, Shader&& fragmentShader);
	ShaderProgram(const std::string vertexFile, const std::string fragmentFile);
	~ShaderProgram();

	inline GLuint GetID() const { return id; }

	virtual void Configure(const Matrix4f& cameraMatrix, const Matrix4f& modelMatrix) = 0;
};

