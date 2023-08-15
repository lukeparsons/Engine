#include "RenderComponent.h"
#include "../../../../core/Global.h"
#include <iostream>

void RenderComponent::FrameUpdate()
{
	mesh.Draw(cameraMatrix, transform->location, transform->rotation, transform->scale);
}
