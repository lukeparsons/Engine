#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <array>
#include "../util/FileIO.h"
#include "../renderer/shaders/Shader.h"
#include "../renderer/shaders/ShaderProgram.h"
#include "../math/Matrixf.h"
#include "../math/Matrix4f.h"
#include "window/Window.h"
#include "../types/Maybe.h"

static const int width = 1024;
static const int height = 576;

static const float aspectRatio = (float)width / float(height);
static const Matrix4f projectionMatrix = GetProjectionMatrix(90.0f, 90.0f, aspectRatio);

static VectorMatrix3f translate = VectorMatrix3f(0.0f, 0.0f, 2.0f);

static const float cameraMoveSpeed = 0.001f;

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

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		translate.z() += cameraMoveSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		translate.z() -= cameraMoveSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		translate.x() += cameraMoveSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		translate.x() -= cameraMoveSpeed;
	}
}

int main()
{
	Matrix4f scaleMatrix = GetScaleMatrix(VectorMatrix3f(0.5f, 0.5f, 0.5f));

	glfwInit();
	// opengl 3.3 (for now)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	Maybe<GLFWwindow*> maybeWindow = ConstructWindow(width, height, "Engine");
	if (!maybeWindow.isJust())
	{
		std::cout << "Failed to construct window" << std::endl;
		return -1;
	}

	GLFWwindow* window = maybeWindow.fromJust();

	// Initalize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initalize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, width, height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	GLfloat vertices[] = {
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f, 
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f, 
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	};

	//unsigned int indices[] = {
	//	0, 1, 3,
	//	1, 2, 3
	//};

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//unsigned int EBO;
	//glGenBuffers(1, &EBO);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	Shader vertexShader = Shader("../Engine/src/shaders/BasicVertex.vertex");
	
	Shader fragmentShader = Shader("../Engine/src/shaders/BasicFragment.fragment");

	ShaderProgram shaderProgram(vertexShader, fragmentShader);

	glUseProgram(shaderProgram.GetID());

	glBindVertexArray(VAO);

	vertexShader.~Shader();
	fragmentShader.~Shader();

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		Matrix4f translateMatrix = GetTranslationMatrix(translate);
		Matrix4f rotationMat = GetXRotationMatrix((float)glfwGetTime());
		rotationMat = rotationMat * GetYRotationMatrix(0.2f * (float)glfwGetTime()) * GetZRotationMatrix((float)glfwGetTime());
		Matrix4f result = projectionMatrix * translateMatrix * rotationMat * scaleMatrix;

		unsigned int transformLoc = glGetUniformLocation(shaderProgram.GetID(), "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_TRUE, GetFlatMatrixf(result));

		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}