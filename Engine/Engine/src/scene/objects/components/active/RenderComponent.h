#pragma once

#include "ActiveComponent.h"
#include "../../../../renderer/Mesh.h"
#include "../static/TransformComponent.h"

class RenderComponent : public ActiveComponent
{
private:
	virtual ActiveComponent* cloneComponent(WorldObject& newWorldObject) const
	{
		return new RenderComponent(*this, newWorldObject);
	}
public:
	const Mesh *const mesh;
	TransformComponent *const transform;

	RenderComponent(const Mesh *const mesh, TransformComponent *const transform) : 
		mesh(mesh), transform(transform) {};

	RenderComponent(const RenderComponent& other, WorldObject& newWorldObject);

	void FrameUpdate();
};

