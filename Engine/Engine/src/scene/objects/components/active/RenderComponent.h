#pragma once

#include "ActiveComponent.h"
#include "../../../../renderer/Mesh.h"
#include "../static/TransformComponent.h"

class RenderComponent : public ActiveComponent
{
public:
	// TODO: Use pointer to mesh?
	Mesh mesh;
	TransformComponent *const transform;

	RenderComponent(const Mesh& mesh, TransformComponent *const transform) : 
		mesh(mesh), transform(transform) {};

	RenderComponent(const RenderComponent& other) : mesh(other.mesh), transform(other.transform) {};

	void FrameUpdate();
};

