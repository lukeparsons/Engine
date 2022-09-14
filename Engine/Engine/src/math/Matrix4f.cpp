#include "Matrix4f.h"
#include "Vector3f.h"

static const float pi = 3.14159265358979323846f;




Matrix4f GetTranslationMatrix(const Vector3f& translation)
{
	return Matrix4f(std::initializer_list<float>
		{	1, 0, 0, translation.x,
			0, 1, 0, translation.y,
			0, 0, 1, translation.z,
			0, 0, 0, 1 });
}

Matrix4f GetScaleMatrix(const Vector3f& scaleVector)
{
	return Matrix4f(std::initializer_list<float>
		{	1, 0, 0, scaleVector.x,
			0, 1, 0, scaleVector.y,
			0, 0, 1, scaleVector.z,
			0, 0, 0, 1 });;
}

Matrix4f GetXRotationMatrix(float angleRadians, const Vector3f& pivot)
{
	float cosAngle = cos(angleRadians);
	float sinAngle = sin(angleRadians);

	return Matrix4f(std::initializer_list<float>
		{	1, 0, 0, 0,
			0, cosAngle, sinAngle, -pivot.z * sinAngle + pivot.y * (1 - cosAngle),
			0, -sinAngle, cosAngle, pivot.y* sinAngle + pivot.z * (1 - cosAngle),
			0, 0, 0, 1 });
}

Matrix4f GetXRotationMatrix(float angleRadians)
{
	float cosAngle = cos(angleRadians);
	float sinAngle = sin(angleRadians);

	return Matrix4f(std::initializer_list<float>
		{	1, 0, 0, 0,
			0, cosAngle, sinAngle,  0,
			0, -sinAngle, cosAngle, 0,
			0, 0, 0, 1 });
}

Matrix4f GetYRotationMatrix(float angle, const Vector3f& pivot)
{
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	return Matrix4f(std::initializer_list<float>
			{	cosAngle, 0, -sinAngle, pivot.z* sinAngle + pivot.x * (1 - cosAngle),
				0, 1, 0, 0,
				sinAngle, 0, cosAngle, -pivot.x * sinAngle + pivot.z * (1 - cosAngle),
				0, 0, 0, 1 });
}

Matrix4f GetYRotationMatrix(float angle)
{
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	return Matrix4f(std::initializer_list<float>
	{	cosAngle, 0, -sinAngle, 0,
		0, 1, 0, 0,
		sinAngle, 0, cosAngle, 0,
		0, 0, 0, 1 });
}

Matrix4f GetZRotationMatrix(float angle, const Vector3f& pivot)
{
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	return Matrix4f(std::initializer_list<float>
			{   cosAngle, -sinAngle, 0, pivot.y * sinAngle + pivot.x * (1 - cosAngle),
				sinAngle,  cosAngle, 0, -pivot.x * sinAngle + pivot.y * (1 - cosAngle),
				0,			0,		1,	0,
				0,			0,		0,	1 });
}

Matrix4f GetZRotationMatrix(float angle)
{
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);

	return Matrix4f(std::initializer_list<float>
	{   cosAngle, -sinAngle, 0, 0,
		sinAngle, cosAngle,  0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 });
}

Matrix4f GetProjectionMatrix(float verticalFOV, float horizontalFOV, float aspectRatio)
{
	verticalFOV /= 2.0f;
	verticalFOV *= (pi / 180);
	float distanceToNearClipPlaneY = 1 / (tanf(verticalFOV));

	horizontalFOV /= 2.0f;
	horizontalFOV *= (pi / 180);
	float distanceToNearClipPlaneZ = 1 / (tanf(horizontalFOV));

	float distanceToFarClipPlane = 50.0f;

	float a = (distanceToFarClipPlane + distanceToNearClipPlaneY) / (distanceToNearClipPlaneY - distanceToFarClipPlane);
	float b = (2 * distanceToFarClipPlane * distanceToNearClipPlaneY) / (distanceToNearClipPlaneY - distanceToFarClipPlane);

	return Matrix4f(std::initializer_list<float>
					{ distanceToNearClipPlaneY / aspectRatio, 0, 0, 0,
					  0, distanceToNearClipPlaneZ,			     0, 0,
					  0, 0,										 a, b,
					  0, 0,										 -1, 0 });
}