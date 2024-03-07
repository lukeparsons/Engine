#include "UIManager.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

void InitUI(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
}

void PrepareFrameUI(std::unique_ptr<Fluid>& fluid, VolumeRender& volRender, std::array<int, 3>* gridsize, float* timeStep, bool* enableLighting)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("FluidController");

	ImGui::SliderFloat("Timestep", timeStep, 0.0f, 2.0f);

	float viscScale = fluid->viscosity * 400000.f;
	ImGui::SliderFloat("Viscosity", &viscScale, 0.0f, 1.0f);
	fluid->viscosity = viscScale / 400000.f;

	float diffScale = fluid->diffusionRate * 600000.f;
	ImGui::SliderFloat("Diffusion Rate", &diffScale , 0.0f, 1.0f);
	fluid->diffusionRate = diffScale / 600000.f;

	ImGui::SliderInt("Iterations", &fluid->max_iterations, 0, 150);

	ImGui::SliderFloat("Smoke Source", &fluid->addsmoke, 0.0f, 5000.f);

	ImGui::SliderFloat("Velocity Source", &fluid->addvel, 0.0f, 5000.f);

	ImGui::SliderFloat3("Box Size", volRender.scale.data(), 1.0f, 10.0f);

	//ImGui::InputInt3("Grid Size", &gridsize);

	if(ImGui::Button("Save"))
	{

	}

	ImGui::Checkbox("Enable Lighting", enableLighting);

	ImGui::ColorEdit3("Light Colour", volRender.volLightingShader->lightColour.data());
	ImGui::ColorEdit3("Sky Colour", volRender.volLightingShader->skyColour.data());

	ImGui::SliderFloat3("Light Vector", volRender.volLightingShader->lightVector.data(), -1.0f, 1.0f);
	ImGui::SliderFloat("Shadow Density", &volRender.volLightingShader->shadowDensity, 0.0f, 256.0f);
	ImGui::SliderFloat("Ambient Density", &volRender.volLightingShader->ambientDensity, 0.0f, 10.0f);

	ImGui::End();
}

void RenderUI()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ShutdownUI()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
