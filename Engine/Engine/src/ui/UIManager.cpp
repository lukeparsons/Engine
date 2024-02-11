#include "UIManager.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

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

void PrepareFrameUI(StableFluids& fluid, VolumeRender& volRender, float* timeStep)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("FluidController");

	ImGui::SliderFloat("Timestep", timeStep, 0.0f, 1.0f);

	float viscScale = fluid.viscosity * 10000;
	ImGui::SliderFloat("Viscosity", &viscScale, 0.0f, 1.0f);
	fluid.viscosity = viscScale / 10000;

	float diffScale = fluid.diffusionRate * 1000;
	ImGui::SliderFloat("Diffusion Rate", &diffScale , 0.0f, 1.0f);
	fluid.diffusionRate = diffScale / 1000;

	ImGui::SliderFloat3("Box Size", volRender.scale.data(), 1, 10);
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
