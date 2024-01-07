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
#include "3DGrid/Grid3D.h"

#define row 20
#define column 20
#define depth 20

/* Task list TODO:
* Advection: Fix halo bicubic interpolation
* Timestep: Add CFL timestep
* PCG: Check
* Boundary condition function: Check
*/

static const int width = 1024;
static const int height = 576;

static const int viewportX = 0;
static const int viewportY = 0;

static constexpr float aspectRatio = (float)width / float(height);
static const Matrix4f projectionMatrix = GetProjectionMatrix(100.0f, 100.0f, aspectRatio);
Matrix4f cameraMatrix;

static Camera camera(Vector3f(0, 0, 0));

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
	glfwSetCursorPosCallback(window, cursor_pos_callback);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	std::shared_ptr<Mesh> box = std::make_shared<Mesh>("../Engine/assets/box.obj");
	//Mesh torus("../Engine/assets/torus.obj", "../Engine/assets/wall2.png", &basicShader);

	std::shared_ptr<Mesh> square = std::make_shared<Mesh>("../Engine/assets/square.obj");

	std::shared_ptr<Texture> wallTex = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/wall2.png"));

	Scene scene;

	Grid3D grid = Grid3D(row, column, depth, scene, Vector3f(0, 0, 0), 1, 0.1f);

	double previousFrameTime = 0;
	float frameTime = 0;
	//float timeStep = 0.0000025;
	while(!glfwWindowShouldClose(window))
	{ 

		double currentFrameTime = glfwGetTime();
		//std::cout << "FPS: " << 60 / (currentFrameTime - previousFrameTime) << std::endl;

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		processInput(window);

		float umax = abs(grid.uVelocity.max()) + sqrt(5 * grid.cellWidth * 9.81f);
		float timeStep = (5 * grid.cellWidth) / grid.uVelocity.max();

		//std::cout << "Time step " << timeStep << std::endl;
		
		grid.advect(timeStep);

		grid.addgravity(timeStep);

		grid.project(timeStep);

		/*for(size_t i = 0; i < row; i++)
		{
			for(size_t j = 0; j < column; j++)
			{
				//grid->PrintCell(i, j);
			}
		} */

		cameraMatrix = projectionMatrix * camera.GetCameraSpaceMatrix() * GetTranslationMatrix(Vector3f(0, 0, -5));

		grid.UpdateRender();

		scene.Update(cameraMatrix);

		glfwSwapBuffers(window);
		glfwPollEvents();

		timeStep = glfwGetTime() - currentFrameTime;
	}
	//std::cout << "Average FPS " << 60 / (frameTime / frameCount);
	glfwTerminate();
	return 0;
}