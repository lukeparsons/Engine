#include "Matrix4f.h"
#include "SquareMatrixf.h"

static const float pi = 3.14159265358979323846f;

static const VectorMatrix3f origin(0.0f, 0.0f, 0.0f);

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

static Matrix4f xRotationMatrix(float cosAngle, float sinAngle, VectorMatrix3f anchor)
{

}

static Matrix4f yRotationMatrix(float cosAngle, float sinAngle, VectorMatrix3f anchor)
{

}

static Matrix4f zRotationMatrix(float cosAngle, float sinAngle, VectorMatrix3f anchor)
{

}

// Angles in radians
Matrix4f GetXRotationMatrix(float angle, VectorMatrix3f anchor)
{
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	Matrix4f rotationMatrix = GetIdentityMatrixf<4>();
	rotationMatrix[1][1] = cosAngle + anchor.x();
	rotationMatrix[1][2] = -sinAngle - anchor.y();
	rotationMatrix[2][1] = sinAngle + anchor.y();
	rotationMatrix[2][2] = cosAngle + anchor.x();

	return rotationMatrix;
}

Matrix4f GetXRotationMatrix(float angle)
{
	return GetXRotationMatrix(angle, origin);
}

Matrix4f GetYRotationMatrix(float angle, VectorMatrix3f anchor)
{
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	Matrix4f rotationMatrix = GetIdentityMatrixf<4>();
	rotationMatrix[0][0] = cosAngle + anchor.x();
	rotationMatrix[0][2] = sinAngle + anchor.y();
	rotationMatrix[2][0] = -sinAngle - anchor.y();
	rotationMatrix[2][2] = cosAngle + anchor.x();

	return rotationMatrix;
}

Matrix4f GetYRotationMatrix(float angle)
{
	return GetYRotationMatrix(angle, origin);
}

Matrix4f GetZRotationMatrix(float angle, VectorMatrix3f anchor)
{
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	Matrix4f rotationMatrix = GetIdentityMatrixf<4>();
	rotationMatrix[0][0] = cosAngle + anchor.x();
	rotationMatrix[0][1] = -sinAngle - anchor.y();
	rotationMatrix[1][0] = sinAngle + anchor.y();
	rotationMatrix[1][1] = cosAngle + anchor.x();

	return rotationMatrix;
}

Matrix4f GetZRotationMatrix(float angle)
{
	return GetZRotationMatrix(angle, origin);
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