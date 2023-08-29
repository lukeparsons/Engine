#pragma once

#include "../../renderer/Mesh.h"
#include "TransformComponent.h"

class RenderComponent : public Component {
private:
public:
	
	const Mesh* mesh;
	const TransformComponent* transform;

	RenderComponent(EntityID _id, std::unordered_map<std::type_index, Component*>& entityComponents) : Component(_id,
		entityComponents)
	{
		mesh = nullptr;
		transform = static_cast<TransformComponent*>(entityComponents[typeid(TransformComponent)]);
	};

	//RenderComponent(EntityID _id, const Mesh* mesh, const TransformComponent* transform) : 
		//ComponentDependent(_id), mesh(mesh), transform(transform) {};
};