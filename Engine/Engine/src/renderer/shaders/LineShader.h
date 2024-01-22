#pragma once
#include "ShaderProgram.h"
#include <array>
#include <algorithm>

class LineShader : public ShaderProgram
{
protected:
	std::array<float, 3> colour = { 0.0f, 0.0f, 0.0f };
	virtual void init()
	{
		transformLoc = glGetUniformLocation(this->id, "transform");
		colourLoc = glGetUniformLocation(this->id, "ourColour");
	}

public:
	unsigned int transformLoc;
	unsigned int colourLoc;
	LineShader(std::string vertexFile, std::string fragmentFile) : ShaderProgram(vertexFile, fragmentFile) 
	{
		init();
	};

	void Configure(const Matrix4f& cameraMatrix, const Matrix4f& modelMatrix) override
	{
		glUniformMatrix4fv(transformLoc, 1, GL_TRUE, cameraMatrix.matrix[0]);
		glUniform3fv(colourLoc, 1, colour.data());
	}

	void SetColour(std::array<float, 3> _colour)
	{
		std::copy(_colour.begin(), _colour.end(), colour.begin());
	}
};