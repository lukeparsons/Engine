#pragma once
#include "../../renderer/shaders/ShaderStore.h"
#include "../../renderer/Mesh.h"
#include "../../scene/camera/Camera.h"
#include <array>

struct VolumeShader : public ShaderProgram
{
	unsigned int transformLoc;
	unsigned int modelLoc;
	unsigned int cameraPosLoc;
	unsigned int smokeLoc;
	unsigned int boxSizeLoc;

	VolumeShader(std::string vertexFile, std::string fragmentFile) : ShaderProgram(vertexFile, fragmentFile)
	{
		transformLoc = glGetUniformLocation(this->id, "transform");
		modelLoc = glGetUniformLocation(this->id, "mesh");

		cameraPosLoc = glGetUniformLocation(this->id, "CameraPos");
		smokeLoc = glGetUniformLocation(this->id, "smoke");
		boxSizeLoc = glGetUniformLocation(this->id, "BoxSize");
	};

	virtual void Configure(const Matrix4f& cameraMatrix, const Matrix4f& modelMatrix) override
	{
		glUseProgram(this->id);
		glUniformMatrix4fv(transformLoc, 1, GL_TRUE, cameraMatrix.matrix[0]);
		glUniformMatrix4fv(modelLoc, 1, GL_TRUE, modelMatrix.matrix[0]);
	}
};

struct VolumeLightingShader : public VolumeShader
{
	std::array<float, 3> lightColour = { 1.0f, 1.0f, 1.0f };
	std::array<float, 3> lightVector = { 0.0f, 0.0f, 0.0f };
	std::array<float, 3> skyColour = { 0.6f, 0.6f, 0.6f };
	float shadowDensity = 16.0f;
	float ambientDensity = 2.0f;

	unsigned int lightColourLoc;
	unsigned int lightVectorLoc;
	unsigned int skyColourLoc;
	unsigned int ambientDensityLoc;
	unsigned int shadowDensityLoc;

	void Configure()
	{
		glUniform3f(lightColourLoc, lightColour[0], lightColour[1], lightColour[2]);
		glUniform3f(lightVectorLoc, lightVector[0], lightVector[1], lightVector[2]);
		glUniform3f(skyColourLoc, skyColour[0], skyColour[1], skyColour[2]);
		glUniform1f(shadowDensityLoc, shadowDensity);
		glUniform1f(ambientDensityLoc, ambientDensity);
	}

	VolumeLightingShader(std::string vertexFile, std::string fragmentFile) : VolumeShader(vertexFile, fragmentFile)
	{
		lightColourLoc = glGetUniformLocation(this->id, "LightColour");
		lightVectorLoc = glGetUniformLocation(this->id, "LightVector");
		skyColourLoc = glGetUniformLocation(this->id, "SkyColour");
		ambientDensityLoc = glGetUniformLocation(this->id, "AmbientDensity");
		shadowDensityLoc = glGetUniformLocation(this->id, "ShadowDensity");
	}
};

class VolumeRender
{
private:
	int column, row, depth;
	std::shared_ptr<Mesh> rect;
	GLuint textureID;
	float* smoke;

	void RenderVolume(VolumeShader* shader);

	void RenderVolumeLighting();

public:
	bool enableLighting;
	std::array<float, 3> scale = { 1.0f, 1.0f, 1.0f };

	std::shared_ptr<VolumeShader> volShader;
	std::shared_ptr<VolumeLightingShader> volLightingShader;

	VolumeRender(int _column, int _row, int _depth, float* _smoke) : column(_column + 2), row(_row + 2), depth(_depth + 2), smoke(_smoke), enableLighting(false)
	{
		rect = std::make_shared<Mesh>("../Engine/assets/box.obj");
		volShader = g_shaderStore.LoadShader<VolumeShader>("../Engine/src/renderer/shaders/shaderfiles/BasicVertex.vertex", "../Engine/src/renderer/shaders/shaderfiles/RayMarch.fragment");
		volLightingShader = g_shaderStore.LoadShader<VolumeLightingShader>("../Engine/src/renderer/shaders/shaderfiles/BasicVertex.vertex", "../Engine/src/renderer/shaders/shaderfiles/RayMarchLighting.fragment");

		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_3D, textureID);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, column, row, depth, 0, GL_RED, GL_FLOAT, smoke);
	}

	void UpdateSize(int _column, int _row, int _depth, float* _smoke)
	{
		column = _column + 2;
		row = _row + 2;
		depth = _depth + 2;
		smoke = _smoke;
		glBindTexture(GL_TEXTURE_3D, textureID);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, column, row, depth, 0, GL_RED, GL_FLOAT, smoke);
	}

	inline void SetShaderValues(VolumeShader* shader, Matrix4f& cameraMatrix, Camera& camera);

	void Render(Matrix4f& cameraMatrix, Camera& camera);
};