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
		for(std::unique_ptr<RenderComponent>& component : renderComponents->GetDenseList())
		{
			if(component->isActive)
			{
				const TransformComponent& transform = transformComponents->GetComponentFromType(component->entity);

				glUseProgram(component->shaderProgram->GetID());

				// TODO: Include rotation matrix
				Matrix4f modelMatrix = GetTranslationMatrix(transform.location) * GetScaleMatrix(transform.scale);
				component->shaderProgram->Configure(cameraMatrix, modelMatrix);

				component->mesh->Draw(cameraMatrix, component->textureID);

				glUseProgram(0);
			}
		}
	}
};