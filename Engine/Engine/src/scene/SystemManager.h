#pragma once
#include <memory>
#include "../scene/ComponentStore.h"
#include "systems/EulerianFluidSystem.h"
#include "systems/RenderSystem.h"

class Matrix4f;
class TransformComponent;
class RenderComponent;

class SystemManager
{
public:

	// This can be non-unique-ptr but RenderSystem (and others) needs to define operator= for that
	std::unique_ptr<RenderSystem> renderSystem;

	// TODO: Can create generic system container for systems that don't require custom parameters
	std::unique_ptr<EulerianFluidSystem> eulerFluidSystem;

	SystemManager(ComponentStore<TransformComponent>* transform, ComponentStore<RenderComponent>* render, ComponentStore<EulerianGridComponent>* euler);
	~SystemManager();

	inline void RunSystems(const Matrix4f& cameraMatrix)
	{
		eulerFluidSystem->Simulate();
		renderSystem->Render(cameraMatrix);
	}
};