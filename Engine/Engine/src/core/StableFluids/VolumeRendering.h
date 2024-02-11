#pragma once
#include "../../renderer/shaders/ShaderStore.h"
#include "../../renderer/Mesh.h"
#include "../../scene/camera/Camera.h"
#include "StableFluids.h"

struct VolumeShader : public ShaderProgram
{
	unsigned int transformLoc;
	unsigned int modelLoc;
	unsigned int windowSizeLoc;
	unsigned int cameraPosLoc;
	unsigned int smokeLoc;
	unsigned int halfBoxSizeLoc;

	VolumeShader(std::string vertexFile, std::string fragmentFile) : ShaderProgram(vertexFile, fragmentFile)
	{
		transformLoc = glGetUniformLocation(this->id, "transform");
		modelLoc = glGetUniformLocation(this->id, "mesh");

		windowSizeLoc = glGetUniformLocation(this->id, "WindowSize");
		cameraPosLoc = glGetUniformLocation(this->id, "CameraPos");
		smokeLoc = glGetUniformLocation(this->id, "smoke");
		halfBoxSizeLoc = glGetUniformLocation(this->id, "halfBoxSize");
	};

	virtual void Configure(const Matrix4f& cameraMatrix, const Matrix4f& modelMatrix) override
	{
		glUseProgram(this->id);
		glUniformMatrix4fv(transformLoc, 1, GL_TRUE, cameraMatrix.matrix[0]);
		glUniformMatrix4fv(modelLoc, 1, GL_TRUE, modelMatrix.matrix[0]);
		glUniform2f(windowSizeLoc, 768, 768);
	}

};

struct VolumeRender
{
	int column, row, depth;
	std::shared_ptr<Mesh> rect;
	GLuint textureID;
	GridStructure<float>* smoke;
	std::array<float, 3> scale = { 1, 1, 1 };
	std::shared_ptr<VolumeShader> volShader;

	VolumeRender(int _column, int _row, int _depth, GridStructure<float>* _smoke) : column(_column + 2), row(_row + 2), depth(_depth + 2), smoke(_smoke)
	{
		rect = std::make_shared<Mesh>("../Engine/assets/box.obj");
		volShader = g_shaderStore.LoadShader<VolumeShader>("../Engine/src/renderer/shaders/shaderfiles/BasicVertex.vertex", "../Engine/src/renderer/shaders/shaderfiles/RayMarch.fragment");

		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_3D, textureID);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, column, row, depth, 0, GL_RED, GL_FLOAT, smoke->grid.data());
	}

	void RenderVolume(Matrix4f& cameraMatrix, Camera& camera);
};