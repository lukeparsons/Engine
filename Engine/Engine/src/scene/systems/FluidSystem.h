#pragma once
#include "EngineSystem.h"
#include "../ComponentStore.h"
#include "../components/TransformComponent.h"
#include "../components/FluidComponent.h"

class FluidSystem : public EngineSystem
{
private:
	ComponentStore<TransformComponent>* const transformComponents;
	ComponentStore<FluidComponent>* const fluidComponents;
public:

	FluidSystem(ComponentStore<TransformComponent>* const _transformComponents, ComponentStore<FluidComponent>* const _fluidComponents)
		: transformComponents(_transformComponents), fluidComponents(_fluidComponents)
	{};

	void Simulate(const Matrix4f& cameraMatrix)
	{
		for(const std::unique_ptr<FluidComponent>& component : fluidComponents->GetDenseList())
		{
			const TransformComponent& transform = transformComponents->GetComponentFromType(component->entity);
			
			glUseProgram(component->fluidShader->GetID());
			Matrix4f modelMatrix = GetTranslationMatrix(transform.location) * GetScaleMatrix(transform.scale);
			component->SetShaderValues(cameraMatrix, modelMatrix);

			component->drawable->Draw(cameraMatrix, component->textureID);
			glUseProgram(0);
		}
	}
};