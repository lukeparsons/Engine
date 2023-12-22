#pragma once
#include "ShaderProgram.h"

class BasicShader : public ShaderProgram
{
protected:
	unsigned int transformLoc;
	unsigned int modelLoc;
	virtual void init()
	{
		transformLoc = glGetUniformLocation(this->id, "transform");
		modelLoc = glGetUniformLocation(this->id, "mesh");
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
	}
};