#include "Window.h"
#include <iostream>

Maybe<GLFWwindow*> ConstructWindow(const int width, const int height, const char* title)
{
	// NULL for monitor (windowed) and NULL for not sharing resources with another window
	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);

	if (window == nullptr)
	{
		glfwTerminate();
		return Maybe<GLFWwindow*>();
	}

	glfwMakeContextCurrent(window);

	return window;
}