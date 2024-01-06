#pragma once
#include <memory>
#include "../scene/ComponentStore.h"
#include "systems/RenderSystem.h"
#include "systems/FluidSystem.h"

class Matrix4f;
class TransformComponent;
class RenderComponent;

class SystemManager
{
public:

	// This can be non-unique-ptr but RenderSystem (and others) needs to define operator= for that
	std::unique_ptr<RenderSystem> renderSystem;

	// TODO: make constructor more general
	SystemManager(ComponentStore<TransformComponent>* const transform, ComponentStore<RenderComponent>* const render);
	~SystemManager();

	inline void RunSystems(const Matrix4f& cameraMatrix) const
	{
		renderSystem->Render(cameraMatrix);
	}
};