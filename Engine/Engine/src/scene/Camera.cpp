#include "Camera.h"
#include <array>

Camera::Camera(Vector3f location)
{
	this->location = location;
	target = Vector3f(0, 0, -1);
	look.x = -90;
}

Matrix4f Camera::GetCameraSpaceMatrix()
{

	Matrix4f translationMatrix = Matrix4f( 
			{ 1, 0, 0, -location.x,
				0, 1, 0, -location.y,
				0, 0, 1, -location.z,
				0, 0, 0, 1 });

	Vector3f U;
	Vector3f V;
	Vector3f N;

	float yaw = look.x * (3.14 / 180);
	float pitch = look.y * (3.14 / 180);

	Vector3f dir;
	dir.x = cos(yaw) * cos(pitch);
	dir.y = sin(pitch);
	dir.z = sin(yaw) * cos(pitch);
	dir.normalise();

	N = dir;
	target = N;

	U = cross(Vector3f(0, 1, 0), N);
	U.normalise();

	V = cross(N, U);

	Matrix4f rotationMatrix = Matrix4f(
				{  U.x, U.y, U.z,   0,
					V.x, V.y, V.z,   0,
					-N.x, -N.y, -N.z, 1,
					0,   0,   0,     1 });

	return rotationMatrix * translationMatrix;
}
