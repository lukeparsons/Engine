#pragma once
#include <GLFW/glfw3.h>
#include "../../types/Maybe.h"

Maybe<GLFWwindow*> ConstructWindow(const int width, const int height, const char* title);
