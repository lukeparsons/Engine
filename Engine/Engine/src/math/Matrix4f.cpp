#include "Matrix4f.h"

Matrix4f GetTranslationMatrix(const Vector3f translation)
{
	Matrix4f translationMatrix;

	translationMatrix[0][3] = translation.x;
	translationMatrix[1][3] = translation.y;
	translationMatrix[2][3] = translation.z;

	return translationMatrix;
}

Matrix4f GetScaleMatrix(const Vector3f scale)
{
	float scaleValues[] = {scale.x, scale.y, scale.z, 1};
	return GetDiagonalMatrixf<4>(scaleValues);
}