#pragma once
#include "Component.h"
#include "../../renderer/Mesh.h"
#include "TransformComponent.h"

class RenderComponent : public Component
{
private:
public:
	const Mesh * mesh;
	const TransformComponent * transform;

	RenderComponent(EntityID _id) : Component(_id) 
	{
		mesh = nullptr;
		transform = nullptr;
	};

	RenderComponent(EntityID _id, const Mesh* mesh, const TransformComponent* transform) : 
		Component(_id), mesh(mesh), transform(transform) {};
};