#pragma once
#include "../../math/Matrix4f.h"
#include <GLFW/glfw3.h>

class Camera {

public:

	Vector3f location;
	Camera(Vector3f location);

	float moveSpeed = 0.008f;
	const float sensitivity = 0.1f;

	Vector3f target;
	Vector3f look;

	void ProcessCameraKeyboardInputs(GLFWwindow* window);
	void ProcessCameraMouseInputs(double Xpos, double Ypos, double prevXpos, double prevYpos);
	Matrix4f GetCameraSpaceMatrix();

};