#include "Camera.h"
#include "../../math/Vectorf.h"
#include <iostream>

Camera::Camera(Vector3f location) : location(location)
{
	target = Vector3f(0, 0, -1);
	look.x = -90;
}

void Camera::ProcessCameraKeyboardInputs(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		location += target * moveSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		location -= target * moveSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		Vector3f left = cross(target, Vector3f(0, 1, 0));
		normalise(left);
		location += left * moveSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		Vector3f right = cross(Vector3f(0, 1, 0), target);
		normalise(right);
		location += right * moveSpeed;
	}
}

void Camera::ProcessCameraMouseInputs(double Xpos, double Ypos, double prevXpos, double prevYpos)
{
	look.x += float((prevXpos - Xpos) * sensitivity);
	look.y += float((prevYpos - Ypos) * sensitivity);
}

Matrix4f Camera::GetCameraSpaceMatrix()
{
	Matrix4f translationMatrix = Matrix4f( 
				1, 0, 0, -location.x,
				0, 1, 0, -location.y,
				0, 0, 1, -location.z,
				0, 0, 0, 1 );

	Vector3f U;
	Vector3f V;
	Vector3f N;

	float yaw = float(look.x * (3.14 / 180));
	float pitch = float(look.y * (3.14 / 180));

	Vector3f dir;
	dir.x = cos(yaw) * cos(pitch);
	dir.y = sin(pitch);
	dir.z = sin(yaw) * cos(pitch);
	normalise(dir);

	N = dir;
	target = N;

	U = cross(Vector3f(0, 1, 0), N);
	normalise(U);

	V = cross(N, U);

	Matrix4f rotationMatrix = Matrix4f(
					U.x, U.y, U.z,   0,
					V.x, V.y, V.z,   0,
					-N.x, -N.y, -N.z, 1,
					0,   0,   0,     1 );

	return rotationMatrix * translationMatrix;
}
