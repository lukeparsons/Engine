#pragma once
#include "ShaderProgram.h"
#include "BasicShader.h"

class FluidShader : public BasicShader
{
private:
	unsigned int coordsLoc;
	unsigned int timestepLoc;
	unsigned int rdxLoc;
	unsigned int uLoc;
	unsigned int xLoc;
	unsigned int bLoc;
	unsigned int alphaLoc;
	unsigned int rBetaLoc;
	unsigned int halfrdxLoc;
	unsigned int wLoc;
	unsigned int pLoc;
protected:
	void init() override
	{
		coordsLoc = glGetUniformLocation(this->id, "coords");
		timestepLoc = glGetUniformLocation(this->id, "timestep");
		rdxLoc = glGetUniformLocation(this->id, "rdx");
		uLoc = glGetUniformLocation(this->id, "u");
		xLoc = glGetUniformLocation(this->id, "x");
		bLoc = glGetUniformLocation(this->id, "b");
		alphaLoc = glGetUniformLocation(this->id, "alpha");
		rBetaLoc = glGetUniformLocation(this->id, "rBeta");
		halfrdxLoc = glGetUniformLocation(this->id, "halfrdx");
		wLoc = glGetUniformLocation(this->id, "w");
		pLoc = glGetUniformLocation(this->id, "p");
	}
public:
	FluidShader(std::string vertexFile, std::string fragmentFile) : BasicShader(vertexFile, fragmentFile) 
	{
		init();
	};

	void Run(const Matrix4f& cameraMatrix, const Matrix4f& modelMatrix, float timestep, float rdx, float coords[2], float u)
	{
		glUniform2fv(coordsLoc, 1, coords);
		glUniform1fv(timestepLoc, 1, &timestep);
		glUniform1fv(rdxLoc, 1, &rdx);
		glUniform1fv(uLoc, 1, &u);

		glUniformMatrix4fv(transformLoc, 1, GL_TRUE, cameraMatrix.matrix[0]);
		glUniformMatrix4fv(modelLoc, 1, GL_TRUE, modelMatrix.matrix[0]);
	}
};