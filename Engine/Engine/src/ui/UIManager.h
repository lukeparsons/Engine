#pragma once
#include "../core/StableFluids/StableFluids.h"
#include <GLFW/glfw3.h>
#include "../core/StableFluids/VolumeRendering.h"

void InitUI(GLFWwindow* window);

void PrepareFrameUI(StableFluids& fluid, VolumeRender& volRender, float* timeStep);

void RenderUI();

void ShutdownUI();