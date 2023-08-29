#pragma once
#include "../../math/Vectorf.h"
#include "Component.h"

class TransformComponent : public Component
{
public:
	Vector3f location;
	Vector3f rotation;
	Vector3f scale;

	TransformComponent(EntityID _id, std::unordered_map<std::type_index, Component*>& entityComponents) : 
		Component(_id, entityComponents), location(Vector3f(0, 0, 0)), scale(Vector3f(1, 1, 1)) {};
};