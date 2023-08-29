#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <array>
#include <vector>
#include <optional>
#include "../util/FileIO.h"
#include "../renderer/shaders/Shader.h"
#include "../renderer/shaders/ShaderProgram.h"
#include "window/Window.h"
#include "../types/Maybe.h"
#include "../scene/Camera/Camera.h"
#include "../math/Matrix4f.h"
#include "../renderer/Mesh.h"
#include "../scene/Scene.h"
#include "../scene/components/RenderComponent.h"

/* Task list TODO:
* Convert std::cout for errors to proper error handling

*/

static const int width = 1024;
static const int height = 576;

static const int viewportX = 0;
static const int viewportY = 0;

static constexpr float aspectRatio = (float)width / float(height);
static const Matrix4f projectionMatrix = GetProjectionMatrix(90.0f, 90.0f, aspectRatio);
Matrix4f cameraMatrix;

static Camera camera(Vector3f(0, 0, 0));

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(viewportX, viewportY, width, height);
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
	// opengl 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// TODO: Should throw an exception, not maybe
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

	glViewport(viewportX, viewportY, width, height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, cursor_pos_callback);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// These shader programs have to stay in scope (for now)
	Shader basicVertexShader("../Engine/src/renderer/shaders/shaderfiles/BasicVertex.vertex");
	Shader basicFragmentShader("../Engine/src/renderer/shaders/shaderfiles/BasicFragment.fragment");
	ShaderProgram basicShader(basicVertexShader, basicFragmentShader);

	Shader fluidVertexShader("../Engine/src/renderer/shaders/shaderfiles/fluids/FluidVertex.vertex");
	Shader fluidFragmentShader("../Engine/src/renderer/shaders/shaderfiles/fluids/FluidFragment.fragment");
	ShaderProgram fluidShader(fluidVertexShader, fluidFragmentShader);

	//Mesh model("box.obj", "wall.tga", "BasicVertex.vertex", "BasicFragment.fragment");
	Mesh model("../Engine/assets/torus.obj", "../Engine/assets/wall2.png", &basicShader);

	//Mesh box("../Engine/assets/box.obj", "../Engine/assets/window.png", &fluidShader);

	Scene scene;
	EntityID e1 = scene.NewEntity();
	RenderComponent* rc = scene.AddComponent<RenderComponent>(e1);
	rc->mesh = &model;
	scene.PrintStatus();
	rc->transform = scene.GetComponent<TransformComponent>(e1);

	//EulerianGrid grid(10, 10, box, Vector3f(-5, -20, 10));

	double previousFrameTime = 0;
	while(!glfwWindowShouldClose(window))
	{

		double currentFrameTime = glfwGetTime();
		//std::cout << "FPS: " << 60 / (currentFrameTime - previousFrameTime) << std::endl;
		previousFrameTime = currentFrameTime;

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		processInput(window);
		
		cameraMatrix = projectionMatrix * camera.GetCameraSpaceMatrix() * GetTranslationMatrix(Vector3f(0, 0, -5));

		scene.Update(cameraMatrix);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}