#pragma once
#include "../core/StableFluids/Fluid.h"
#include "../core/StableFluids/VolumeRendering.h"

void InitUI(GLFWwindow* window);

bool PrepareFrameUI(std::unique_ptr<Fluid>& fluid, VolumeRender& volRender, std::array<int, 3>* gridsize, float* timeStep, bool* enableLighting);

void RenderUI();

void ShutdownUI();