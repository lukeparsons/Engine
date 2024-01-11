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
#include "StableFluids/Test.h"
#include "StableFluids/StableFluids.h"
#include "../renderer/Line.h"

#define row 24
#define column 24
#define depth 24

/* Task list TODO:
* Advection: Fix halo bicubic interpolation
* Timestep: Add CFL timestep
* PCG: Check
* Boundary condition function: Check
*/

static const int width = 768;
static const int height = 768;

static const int viewportX = 0;
static const int viewportY = 0;

static constexpr float aspectRatio = (float)width / float(height);
static const Matrix4f projectionMatrix = GetProjectionMatrix(90.0f, 90.0f, aspectRatio);
Matrix4f cameraMatrix;

static Camera camera(Vector3f(0, 0, 0));

static bool addForceU, addForceV, addForceW = false;

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

static const std::shared_ptr<int> h = std::make_shared<int>(10);
static const std::shared_ptr<int> m = std::make_shared<int>(20);


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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	
	std::shared_ptr<Mesh> box = std::make_shared<Mesh>("../Engine/assets/box.obj");
	//Mesh torus("../Engine/assets/torus.obj", "../Engine/assets/wall2.png", &basicShader);

	std::shared_ptr<Mesh> square = std::make_shared<Mesh>("../Engine/assets/square.obj");

	std::shared_ptr<Texture> wallTex = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/wall2.png"));

	Scene scene;

	//StableFluids fluid = StableFluids(row, column, depth, scene, Vector3f(0, 0, 0), 0.0f, 0.01f);


	double previousFrameTime = 0;
	float frameTime, uMax = 0;
	float timeStep = 0.4f;

	//scene.CreateLine(std::make_shared<Line>(0.25f, 0.f, 0.25f, 1.0f, 0.f, 0.25f));
	//scene.CreateLine(std::make_shared<Line>(0.25f, 0.f, 0.25f, 0.25f, 1.f, 0.25f));
	//scene.CreateLine(std::make_shared<Line>(0.25f, 0.f, 0.25f, 0.25f, 0.f, 1.f));

	initsim(scene);
	while(!glfwWindowShouldClose(window))
	{ 

		double currentFrameTime = glfwGetTime();
		//std::cout << "FPS: " << 60 / (currentFrameTime - previousFrameTime) << std::endl;

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		processInput(window);

		cameraMatrix = projectionMatrix * camera.GetCameraSpaceMatrix() * GetTranslationMatrix(Vector3f(0, 0, -3));

		//fluid.Simulate(timeStep, 0.0f);

		sim_main(addForceU, addForceV, addForceW);

		sim_draw();

		//fluid.UpdateRender();

		scene.Update(cameraMatrix);

		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	//std::cout << "Average FPS " << 60 / (frameTime / frameCount);
	glfwTerminate();
	return 0;
}