#pragma once
#include "../WorldObject.h"
#include "../../math/Matrix4f.h"
#include <GLFW/glfw3.h>

class Camera : public WorldObject {

public:

	Camera(Vector3f location);

	const float moveSpeed = 0.001f;
	const float sensitivity = 0.1f;

	Vector3f target;
	Vector3f look;

	void ProcessCameraKeyboardInputs(GLFWwindow* window);
	void ProcessCameraMouseInputs(double Xpos, double Ypos, double prevXpos, double prevYpos);
	Matrix4f GetCameraSpaceMatrix();

};