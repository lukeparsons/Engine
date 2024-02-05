#include "VolumeRendering.h"

void VolumeRender::RenderVolume(Matrix4f& cameraMatrix, Camera& camera)
{
	Matrix4f modelMatrix = GetTranslationMatrix(Vector3f(0, 0, 0)) * GetScaleMatrix({ scale[0], scale[1], scale[2] });
	volShader->Configure(cameraMatrix, modelMatrix);

	glUniform3f(volShader->halfBoxSizeLoc, scale[0], scale[1], scale[2]);

	glUniform3f(volShader->cameraPosLoc, camera.location.x, camera.location.y, camera.location.z);

	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, column, row, depth, GL_RED, GL_FLOAT, smoke->grid.data());
	glUniform1i(volShader->smokeLoc, 0);
	rect->QuickDraw();
}