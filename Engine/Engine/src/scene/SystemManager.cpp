#include "SystemManager.h"

SystemManager::SystemManager(ComponentStore<TransformComponent>* const transform, ComponentStore<RenderComponent>* const render, ComponentStore<FluidComponent>* const fluid)
{
	renderSystem = std::make_unique<RenderSystem>(transform, render);
	fluidSystem = std::make_unique<FluidSystem>(transform, fluid);
}

SystemManager::~SystemManager() = default;
