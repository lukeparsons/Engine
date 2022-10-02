#include "Vector3f.h"
#include <cmath>
#include <iostream>

Vector3f& normalise(Vector3f& vector)
{
	float length = sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);

	vector.x /= length;
	vector.y /= length;
	vector.z /= length;

	return vector;
}

Vector3f cross(const Vector3f& left, const Vector3f& right)
{
	float x = left.y * right.z - left.z * right.y;
	float y = left.z * right.x - left.x * right.z;
	float z = left.x * right.y - left.y * right.x;
	return Vector3f(x, y, z);
}

void PrintVector(const Vector3f& vector)
{
	std::cout << vector.x << ", " << vector.y << ", " << vector.z << std::endl;
}