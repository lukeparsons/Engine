#pragma once

#include "EngineSystem.h"
#include "../components/EulerianGridComponent.h"
#include "../components/TransformComponent.h"
#include "../ComponentStore.h"
#include <GLFW/glfw3.h>

/* Simulating incompressible, homogeneous fluids (constant volume, constant density in both time and space)
*/
class EulerianFluidSystem : public EngineSystem
{
private:
	ComponentStore<TransformComponent>* const transformComponents;
	ComponentStore<EulerianGridComponent>* const eulerianComponents;

public:

	EulerianFluidSystem(ComponentStore<TransformComponent>* const _transformComponents,
		ComponentStore<EulerianGridComponent>* const _eulerianComponents) : transformComponents(_transformComponents), eulerianComponents(_eulerianComponents) {};

	void Simulate()
	{
		for(std::unique_ptr<EulerianGridComponent>& eulerComponent : eulerianComponents->GetDenseList())
		{

			for(Cell& cell : eulerComponent->cells)
			{
				float colour = float(sin(glfwGetTime() / 2.0f) + 0.5f);
				glUseProgram(cell.mesh->shaderProgram->GetID());
				glUniform4f(cell.colourLoc, 0.0f, colour, 0.0f, 1.0f);
				glUseProgram(0);
			}
		}
	}
};