#pragma once
#include "EngineSystem.h"
#include "../ComponentStore.h"
#include "../components/RenderComponent.h"
#include "../Scene.h"
#include <iostream>

class RenderSystem : public EngineSystem
{
private:
	ComponentStore<TransformComponent>* const transformComponents;
	ComponentStore<RenderComponent>* const renderComponents;

public:

	RenderSystem(ComponentStore<TransformComponent>* const _transformComponents, ComponentStore<RenderComponent>* const _renderComponents)
		: transformComponents(_transformComponents), renderComponents(_renderComponents) {};

	void Render(const Matrix4f& cameraMatrix)
	{
		for(RenderComponent& component : renderComponents->GetDenseList())
		{
			TransformComponent& transform = transformComponents->GetComponentFromType(component.entity);
			component.mesh->Draw(cameraMatrix, transform.location, transform.rotation, transform.scale);
		}
	}
};