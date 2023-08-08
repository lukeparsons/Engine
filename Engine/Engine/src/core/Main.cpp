#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <array>
#include "../util/FileIO.h"
#include "../renderer/shaders/Shader.h"
#include "../renderer/shaders/ShaderProgram.h"
#include "window/Window.h"
#include "../types/Maybe.h"
#include "../scene/Camera/Camera.h"
#include "../util/formats/TGA.h"
#include <vector>
#include "../math/Matrix4f.h"
#include "../renderer/Mesh.h"
#include "../scene/RenderedObject.h"

static const int width = 1024;
static const int height = 576;

static constexpr float aspectRatio = (float)width / float(height);
static const Matrix4f projectionMatrix = GetProjectionMatrix(90.0f, 90.0f, aspectRatio);

static Camera camera(Vector3f(0, 0, 0));

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	camera.ProcessCameraKeyboardInputs(window);
}

double pxpos, pypos;
bool firstMouse = true;
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		pxpos = xpos;
		pypos = ypos;
		firstMouse = false;
	}
	
	camera.ProcessCameraMouseInputs(xpos, ypos, pxpos, pypos);

	pxpos = xpos;
	pypos = ypos;
}

int main()
{

	glfwInit();
	// opengl 3.3 (for now)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	Maybe<GLFWwindow*> maybeWindow = ConstructWindow(width, height, "Engine");
	if(!maybeWindow.isJust())
	{
		std::cout << "Failed to construct window" << std::endl;
		return -1;
	}

	GLFWwindow* window = maybeWindow.fromJust();

	// Initalize GLAD
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initalize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, width, height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, cursor_pos_callback);

	glEnable(GL_DEPTH_TEST);

	Shader basicVertexShader("../Engine/src/shaders/BasicVertex.vertex");
	Shader basicFragmentShader("../Engine/src/shaders/BasicFragment.fragment");
	ShaderProgram basicShader(basicVertexShader, basicFragmentShader);

	//Mesh model("box.obj", "wall.tga", "BasicVertex.vertex", "BasicFragment.fragment");
	Mesh model("../Engine/assets/torus.obj", "../Engine/assets/wall.tga", basicShader);
	RenderedObject doughnut(model);
	doughnut.scale = Vector3f(1, 1, 1);

	Mesh box("../Engine/assets/box.obj", "../Engine/assets/wall.tga", basicShader);
	RenderedObject boxobj(box, Vector3f(1, 1, 1));
	boxobj.scale = Vector3f(3, 3, 3);

	Matrix4f translateMatrix = GetTranslationMatrix(Vector3f(0, 0, -5));

	double previousFrameTime = 0;
	while(!glfwWindowShouldClose(window))
	{

		double currentFrameTime = glfwGetTime();
		//std::cout << "FPS: " << 60 / (currentFrameTime - previousFrameTime) << std::endl;
		previousFrameTime = currentFrameTime;

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		processInput(window);

		Matrix4f cameraMatrix = camera.GetCameraSpaceMatrix();
		
		Matrix4f result = projectionMatrix * cameraMatrix * translateMatrix;

		doughnut.DrawObject(result);
		boxobj.DrawObject(result);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}