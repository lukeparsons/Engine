#include "SystemManager.h"

SystemManager::SystemManager(ComponentStore<TransformComponent>* const transform, ComponentStore<RenderComponent>* const render)
{
	renderSystem = std::make_unique<RenderSystem>(transform, render);
}

SystemManager::~SystemManager() = default;
