#pragma once
#include <memory>
#include "../components/TransformComponent.h"

class WorldObject
{
private:
	TransformComponent transform;
public:

	unsigned short id;

	//WorldObject(const TransformComponent& transform = TransformComponent()) : transform(transform) {};

};

//WorldObject CreateModel(const Mesh& model, const TransformComponent& transform = TransformComponent());