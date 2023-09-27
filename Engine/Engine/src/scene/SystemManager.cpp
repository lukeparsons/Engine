#include "SystemManager.h"

SystemManager::SystemManager(ComponentStore<TransformComponent>* const transform, ComponentStore<RenderComponent>* const render, ComponentStore<EulerianGridComponent>* euler)
{
	renderSystem = std::make_unique<RenderSystem>(transform, render);

	eulerFluidSystem = std::make_unique<EulerianFluidSystem>(transform, euler);
}

SystemManager::~SystemManager() = default;
