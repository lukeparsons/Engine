#pragma once
#include "EngineSystem.h"
#include "../components/RenderComponent.h"

class RenderSystem : public EngineSystem<RenderComponent>
{
public:
	void Render(const Matrix4f& cameraMatrix)
	{
		for(RenderComponent& component : this->dense)
		{
			component.mesh->Draw(cameraMatrix, component.transform->location, component.transform->rotation, component.transform->scale);
		}
	}
};