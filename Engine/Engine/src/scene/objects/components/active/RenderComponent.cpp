#include "RenderComponent.h"
#include "../../../../core/Global.h"
#include "../../WorldObject.h"
#include <iostream>

RenderComponent::RenderComponent(const RenderComponent& other, WorldObject& newWorldObject) : mesh(other.mesh), transform(&newWorldObject.transform) {};

void RenderComponent::FrameUpdate()
{
	mesh->Draw(cameraMatrix, transform->location, transform->rotation, transform->scale);
}
