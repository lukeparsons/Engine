#include "Matrix4f.h"

static const float pi = 3.14159265358979323846f;

Matrix4f GetTranslationMatrix(const Vector3f& translation)
{
	Matrix4f translationMatrix = GetIdentityMatrixf<4>();

	translationMatrix[0][3] = translation.x;
	translationMatrix[1][3] = translation.y;
	translationMatrix[2][3] = translation.z;

	return translationMatrix;
}

Matrix4f GetScaleMatrix(const Vector3f& scale)
{
	float scaleValues[] = { scale.x, scale.y, scale.z, 1 };
	return GetDiagonalMatrixf<4>(scaleValues);
}

// Angles in radians
Matrix4f GetXRotationMatrix(float angle)
{
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	Matrix4f rotationMatrix = GetIdentityMatrixf<4>();
	rotationMatrix[1][1] = cosAngle;
	rotationMatrix[1][2] = -sinAngle;
	rotationMatrix[2][1] = sinAngle;
	rotationMatrix[2][2] = cosAngle;

	return rotationMatrix;
}

Matrix4f GetYRotationMatrix(float angle)
{
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	Matrix4f rotationMatrix = GetIdentityMatrixf<4>();
	rotationMatrix[0][0] = cosAngle;
	rotationMatrix[0][2] = sinAngle;
	rotationMatrix[2][0] = -sinAngle;
	rotationMatrix[2][2] = cosAngle;

	return rotationMatrix;
}

Matrix4f GetZRotationMatrix(float angle)
{
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	Matrix4f rotationMatrix = GetIdentityMatrixf<4>();
	rotationMatrix[0][0] = cosAngle;
	rotationMatrix[0][1] = -sinAngle;
	rotationMatrix[1][0] = sinAngle;
	rotationMatrix[1][1] = cosAngle;

	return rotationMatrix;
}

Matrix4f GetProjectionMatrix(float verticalFOV)
{
	verticalFOV /= 2.0f;
	verticalFOV *= (pi / 180);
	float distanceToNearClipPlane = 1 / (tanf(verticalFOV));

	float diagValues[] = { distanceToNearClipPlane, distanceToNearClipPlane, 1, 1 };

	Matrix4f projectionMatrix = GetDiagonalMatrixf<4>(diagValues);

	// temp
	projectionMatrix[3][3] = 0;
	projectionMatrix[3][2] = 1;

	return projectionMatrix;
}

VectorMatrix ApplyTransformationMatrix(const Matrix4f& transformation, const Vector3f& vector)
{
	VectorMatrix vectorMatrix;
	vectorMatrix[0][0] = vector.x;
	vectorMatrix[1][0] = vector.y;
	vectorMatrix[2][0] = vector.z;
	vectorMatrix[3][0] = 1;

	return transformation * vectorMatrix;
}