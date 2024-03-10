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
#include "../math/Matrix.h"
#include "../renderer/Texture.h"
#include "../renderer/shaders/BasicShader.h"
#include "../renderer/shaders/FluidShader.h"
#include "../renderer/shaders/ShaderStore.h"
#include "StableFluids/CPUFluid/StableFluids.h"
#include "../renderer/Line.h"
#include "StableFluids/VolumeRendering.h"
#include "../ui/UIManager.h"
#include "StableFluids/OpenCL/OpenCLFluids.h"
#include "StableFluids/OldOpenCL/OldOpenCLFluids.h"

#define row 220
#define column 220
#define depth 220

static const int width = 1920;
static const int height = 1080;

static const int viewportX = 0;
static const int viewportY = 0;

static constexpr float aspectRatio = (float)width / float(height);
static const Matrix4f projectionMatrix = GetProjectionMatrix(90.0f, 90.0f, aspectRatio);
Matrix4f cameraMatrix;

static Camera camera(Vector3f(0, 0, 5));

static bool addForceU, addForceV, addForceW, negaddForceU, negaddForceV, negaddForceW, addSmoke, clear = false;
static bool cursorShown, forceMode = false;
static float xForceMotion, yForceMotion = 0.0f;

ShaderStore g_shaderStore = ShaderStore();

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

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		addForceU = true;
	}

	if(key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		addForceV = true;
	}

	if(key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		addForceW = true;
	}

	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		negaddForceU = true;
	}

	if (key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		negaddForceV = true;
	}

	if (key == GLFW_KEY_B && action == GLFW_PRESS)
	{
		negaddForceW = true;
	}

	if(key == GLFW_KEY_K && action == GLFW_PRESS)
	{
		addSmoke = true;
	}

	if(key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		clear = true;
	}

	if(key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		forceMode = !forceMode;
		xForceMotion = 0.0f;
		yForceMotion = 0.0f;
	}
}

double pxpos, pypos;
bool firstMouse = true;
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if(firstMouse)
	{
		pxpos = xpos;
		pypos = ypos;
		firstMouse = false;
	}

	if(forceMode)
	{
		xForceMotion = xpos - pxpos;
		yForceMotion = ypos - pypos;
	}

	if(!cursorShown)
	{
		camera.ProcessCameraMouseInputs(xpos, ypos, pxpos, pypos);
	}


	pxpos = xpos;
	pypos = ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		cursorShown = !cursorShown;
		cursorShown ? glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL) : glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
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
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	std::shared_ptr<Mesh> box = std::make_shared<Mesh>("../Engine/assets/box.obj");

	//InitUI(window);

	std::array<int, 3> gridsize = { 80, 80, 80 };
	std::unique_ptr<Fluid> fluid = std::make_unique<OpenCLFluids>(column, row, depth, 10);

	double currentFrameTime = 0;
	double frameTime = 0.f;
	unsigned int frameCount = 1000;
	float timeStep = 0.4f;
	float f, f2, f3, f4;

	VolumeRender volRender = VolumeRender(column, row, depth, fluid->GetSmokeData());
	Matrix4f cameraSpaceMatrix = camera.GetCameraSpaceMatrix();
	for(int i = 0; i < frameCount; i++)
	{
		currentFrameTime = glfwGetTime();
		//std::cout << "FPS: " << 60 / (currentFrameTime - previousFrameTime) << std::endl;

		glClearColor(0.5f, 0.4f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/*if(PrepareFrameUI(fluid, volRender, &gridsize, &timeStep, &volRender.enableLighting))
		{
			fluid = std::make_unique<OpenCLFluids>(gridsize[0], gridsize[1], gridsize[2]);
			volRender.UpdateSize(gridsize[0], gridsize[1], gridsize[2], fluid->GetSmokeData());
		} */

		processInput(window);

		cameraSpaceMatrix = camera.GetCameraSpaceMatrix();

		cameraMatrix = projectionMatrix * cameraSpaceMatrix;

		//fluid->Simulate(timeStep, addForceU, addForceV, addForceW, negaddForceU, negaddForceV, negaddForceW, addSmoke, clear);

		f = (float)i;
		f2 = f / 2.0f;
		f3 = f / 3.0f;
		f4 = f / 4.0f;
		fluid->Profile(0.4f, f, -f2, f3, -f, f4, -f3, f2);

		volRender.Render(cameraMatrix, camera);
	
		//RenderUI();

		glfwSwapBuffers(window);
		glfwPollEvents();
		frameTime += glfwGetTime() - currentFrameTime;
	}

	std::cout << "Average Frametime " << (frameTime / (double)frameCount) << std::endl;
	//ShutdownUI();
	glfwTerminate();

	return 0;
}