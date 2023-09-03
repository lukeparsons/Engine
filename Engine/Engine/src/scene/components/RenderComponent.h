#pragma once

#include "../../renderer/Mesh.h"
#include "TransformComponent.h"

class RenderComponent : public Component {
private:
public:
	
	const Mesh* mesh;

	RenderComponent(EntityID _id) : Component(_id), mesh(nullptr) {};
};