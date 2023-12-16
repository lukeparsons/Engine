#pragma once
#include "ShaderProgram.h"

class BasicShader : public ShaderProgram
{
public:

	BasicShader() : ShaderProgram("../Engine/src/renderer/shaders/shaderfiles/BasicVertex.vertex", "../Engine/src/renderer/shaders/shaderfiles/BasicFragment.fragment") {};

	void Configure(const Matrix4f& cameraMatrix, const Matrix4f& modelMatrix) override
	{
		glUniformMatrix4fv(transformLoc, 1, GL_TRUE, cameraMatrix.matrix[0]);
		glUniformMatrix4fv(modelLoc, 1, GL_TRUE, modelMatrix.matrix[0]);
	}
};