#include "Camera.h"

Camera::Camera(Vector3f location)
{
	this->location = location;
	target = Vector3f(0, 0, -1);
	look.x = -90;
}

Matrix4f Camera::GetCameraSpaceMatrix()
{

	float translationValues[] = { 1, 0, 0, -location.x,
								  0, 1, 0, -location.y,
								  0, 0, 1, -location.z,
								  0, 0, 0, 1 };

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

	std::cout << yaw << std::endl;

	N = dir;
	target = N;

	U = cross(Vector3f(0, 1, 0), N);
	U.normalise();

	V = cross(N, U);

	/*std::cout << std::endl;
	Print(U);
	std::cout << std::endl;
	Print(V);
	std::cout << std::endl;
	Print(N);*/

	float rotationValues[] = {  U.x, U.y, U.z, 0,
								V.x, V.y, V.z, 0,
							   -N.x, -N.y, -N.z, 1,
								0,   0,   0,   1 };

	return Matrix4f(rotationValues) * Matrix4f(translationValues);
}
