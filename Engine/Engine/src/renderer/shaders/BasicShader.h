#pragma once
#include "ShaderProgram.h"
#include <array>
#include <algorithm>

class BasicShader : public ShaderProgram
{
protected:
	unsigned int transformLoc;
	unsigned int modelLoc;
	unsigned int colourLoc;
	std::array<float, 3> colour = { 0.0f, 0.0f, 0.0f };
	virtual void init()
	{
		transformLoc = glGetUniformLocation(this->id, "transform");
		modelLoc = glGetUniformLocation(this->id, "mesh");
		colourLoc = glGetUniformLocation(this->id, "ourColour");
	}

public:

	BasicShader(std::string vertexFile, std::string fragmentFile) : ShaderProgram(vertexFile, fragmentFile) 
	{
		init();
	};

	void Configure(const Matrix4f& cameraMatrix, const Matrix4f& modelMatrix) override
	{
		glUniformMatrix4fv(transformLoc, 1, GL_TRUE, cameraMatrix.matrix[0]);
		glUniformMatrix4fv(modelLoc, 1, GL_TRUE, modelMatrix.matrix[0]);
		glUniform3fv(colourLoc, 1, colour.data());
	}

	void SetColour(std::array<float, 3> _colour)
	{
		std::copy(_colour.begin(), _colour.end(), colour.begin());
	}
};