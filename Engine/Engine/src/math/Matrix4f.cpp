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

// Angles in radians
Matrix4f GetXRotationMatrix(float angle, VectorMatrix3f pivot)
{
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	float values[] = { 1, 0,		0,		  0,
					   0, cosAngle, sinAngle, -pivot.z() * sinAngle + pivot.y() * (1 - cosAngle),
					   0, -sinAngle, cosAngle, pivot.y() * sinAngle + pivot.z() * (1 - cosAngle),
					   0, 0,		0,		  1
	};

	return Matrix4f(values);
}

Matrix4f GetXRotationMatrix(float angle)
{
	return GetXRotationMatrix(angle, origin);
}

Matrix4f GetYRotationMatrix(float angle, VectorMatrix3f pivot)
{
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	float values[] = { cosAngle, 0, -sinAngle, pivot.z() * sinAngle + pivot.x() * (1 - cosAngle),
					   0,		 1,	0,		   0,
					   sinAngle, 0, cosAngle,  -pivot.x() * sinAngle + pivot.z() * (1 - cosAngle),
					   0,		 0, 0,		   1
	};

	return Matrix4f(values);
}

Matrix4f GetYRotationMatrix(float angle)
{
	return GetYRotationMatrix(angle, origin);
}

Matrix4f GetZRotationMatrix(float angle, VectorMatrix3f pivot)
{
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	float values[] = { cosAngle, -sinAngle, 0, pivot.y() * sinAngle + pivot.x() * (1 - cosAngle),
					   sinAngle,  cosAngle, 0, -pivot.x() * sinAngle + pivot.y() * (1 - cosAngle),
						0,			0,		1,	0,
						0,			0,		0,	1
	};

	return Matrix4f(values);
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