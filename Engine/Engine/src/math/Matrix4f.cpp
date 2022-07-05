#include "Matrix4f.h"
#include "SquareMatrixf.h"

static const float pi = 3.14159265358979323846f;

Matrix4f GetTranslationMatrix(const VectorMatrix3f& translation)
{
	Matrix4f translationMatrix = GetIdentityMatrixf<4>();

	translationMatrix[0][3] = translation.matrix[0][0];
	translationMatrix[1][3] = translation.matrix[0][1];
	translationMatrix[2][3] = translation.matrix[0][2];

	return translationMatrix;
}

Matrix4f GetScaleMatrix(const VectorMatrix3f& scaleVector)
{
	const float* scaleValues = scaleVector.matrix[0];
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

Matrix4f GetProjectionMatrix(float verticalFOV, float horizontalFOV, float aspectRatio)
{
	verticalFOV /= 2.0f;
	verticalFOV *= (pi / 180);
	float distanceToNearClipPlaneY = 1 / (tanf(verticalFOV));

	horizontalFOV /= 2.0f;
	horizontalFOV *= (pi / 180);
	float distanceToNearClipPlaneZ = 1 / (tanf(horizontalFOV));

	float distanceToFarClipPlane = 10.0f;

	float a = (-distanceToFarClipPlane - distanceToNearClipPlaneY) / (distanceToNearClipPlaneY - distanceToFarClipPlane);
	float b = (2 * distanceToFarClipPlane * distanceToNearClipPlaneY) / (distanceToNearClipPlaneY - distanceToFarClipPlane);

	float values[] = { distanceToNearClipPlaneY / aspectRatio, 0, 0, 0,
					  0, distanceToNearClipPlaneZ,			      0, 0,
					  0, 0,										  a, b,
					  0, 0,										  1, 0 };

	Matrix4f projectionMatrix = values;

	return projectionMatrix;
}