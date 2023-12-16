#pragma once
#include "ShaderProgram.h"

class FluidShader : public ShaderProgram
{
public:

	FluidShader() : ShaderProgram("../Engine/src/renderer/shaders/shaderfiles/fluids/FluidVertex.vertex", "../Engine/src/renderer/shaders/shaderfiles/fluids/FluidFragment.fragment") {};

	void Configure(const Matrix4f& cameraMatrix, const Matrix4f& modelMatrix) override
	{
		glUniformMatrix4fv(transformLoc, 1, GL_TRUE, cameraMatrix.matrix[0]);
		glUniformMatrix4fv(modelLoc, 1, GL_TRUE, modelMatrix.matrix[0]);
	}
};