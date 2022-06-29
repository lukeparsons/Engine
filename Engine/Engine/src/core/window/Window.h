#pragma once
#include <GLFW/glfw3.h>
#include <optional>

std::optional<GLFWwindow*> ConstructWindow(const int width, const int height, const char* title);
