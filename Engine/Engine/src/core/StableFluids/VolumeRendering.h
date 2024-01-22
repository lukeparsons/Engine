#pragma once
#include "../../renderer/shaders/ShaderStore.h"
#include "../../renderer/Mesh.h"

struct VolumeShader : public ShaderProgram
{
	unsigned int transformLoc;
	unsigned int modelLoc;
	unsigned int projectionLoc;
	unsigned int cameraSpaceLoc;
	unsigned int windowSizeLoc;
	unsigned int cameraPosLoc;
	unsigned int cameraZTargetLoc;
	unsigned int smokeLoc;

	VolumeShader(std::string vertexFile, std::string fragmentFile) : ShaderProgram(vertexFile, fragmentFile)
	{
		transformLoc = glGetUniformLocation(this->id, "transform");
		modelLoc = glGetUniformLocation(this->id, "mesh");

		projectionLoc = glGetUniformLocation(this->id, "projection");
		cameraSpaceLoc = glGetUniformLocation(this->id, "cameraSpaceLOc");
		windowSizeLoc = glGetUniformLocation(this->id, "WindowSize");
		cameraPosLoc = glGetUniformLocation(this->id, "CameraPos");
		cameraZTargetLoc = glGetUniformLocation(this->id, "CameraZTarget");
		smokeLoc = glGetUniformLocation(this->id, "smoke");
	};

	virtual void Configure(const Matrix4f& cameraMatrix, const Matrix4f& modelMatrix) override
	{
		glUseProgram(this->id);
		glUniformMatrix4fv(transformLoc, 1, GL_TRUE, cameraMatrix.matrix[0]);
		glUniformMatrix4fv(modelLoc, 1, GL_TRUE, modelMatrix.matrix[0]);
		glUniform2f(windowSizeLoc, 768, 768);
	}
};

static std::shared_ptr<VolumeShader> volRender;
static std::shared_ptr<Mesh> rect;
static std::shared_ptr<Texture> texture;
static GLuint textureID;

static int column, row, depth;

void InitVolumeRender(int _column, int _row, int _depth, GridStructure<float>& smoke)
{
	volRender = g_shaderStore.LoadShader<VolumeShader>("../Engine/src/renderer/shaders/shaderfiles/BasicVertex.vertex", "../Engine/src/renderer/shaders/shaderfiles/RayMarch.fragment");
	rect = std::make_shared<Mesh>("../Engine/assets/box.obj");

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_3D, textureID);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	column = _column;
	row = _row;
	depth = _depth;

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, column, row, depth, 0, GL_RED, GL_FLOAT, smoke.grid.data());
}

void VolumeRender(Matrix4f& cameraMatrix, const Matrix4f& projectionMatrix, const Matrix4f& cameraSpaceMatrix, Camera& camera, GridStructure<float>& smoke)
{
	Matrix4f modelMatrix = GetTranslationMatrix(Vector3f(0, 0, 0)) * GetScaleMatrix(Vector3f(1, 1, 1));
	volRender->Configure(cameraMatrix, modelMatrix);

	glUniform3f(volRender->cameraPosLoc, camera.location.x, camera.location.y, camera.location.z);
	glUniform1f(volRender->cameraZTargetLoc, camera.target.z);
	glUniformMatrix4fv(volRender->cameraSpaceLoc, 1, GL_TRUE, cameraSpaceMatrix.matrix[0]);
	glUniformMatrix4fv(volRender->projectionLoc, 1, GL_TRUE, projectionMatrix.matrix[0]);

	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, column, row, depth, GL_RED, GL_FLOAT, smoke.grid.data());
	glUniform1i(volRender->smokeLoc, 0);
	rect->QuickDraw();
}