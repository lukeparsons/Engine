#include "VolumeRendering.h"

inline void VolumeRender::SetShaderValues(VolumeShader* shader, Matrix4f& cameraMatrix, Camera& camera)
{
	Matrix4f modelMatrix = GetTranslationMatrix(Vector3f(0, 0, 0)) * GetScaleMatrix({ scale[0], scale[1], scale[2] });
	shader->Configure(cameraMatrix, modelMatrix);
	glUniform3f(shader->cameraPosLoc, camera.location.x, camera.location.y, camera.location.z);
}

void VolumeRender::Render(Matrix4f& cameraMatrix, Camera& camera) {

	if(enableLighting)
	{
		SetShaderValues(volLightingShader.get(), cameraMatrix, camera);
		RenderVolumeLighting();
		RenderVolume(volLightingShader.get());
	} else
	{
		SetShaderValues(volShader.get(), cameraMatrix, camera);
		RenderVolume(volShader.get());
	};
}

void VolumeRender::RenderVolume(VolumeShader* shader)
{
	glUniform3f(shader->boxSizeLoc, scale[0], scale[1], scale[2]);

	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, column, row, depth, GL_RED, GL_FLOAT, smoke);
	glUniform1i(shader->smokeLoc, 0);
	rect->QuickDraw();
}

void VolumeRender::RenderVolumeLighting()
{
	volLightingShader->Configure();

}