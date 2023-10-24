#pragma once
#include "../../renderer/Mesh.h"
#include "../../scene/Scene.h"
#include "../../scene/components/RenderComponent.h"
#include <map>
#include "../../math/Matrix.h"

class Cell2D
{
public:
	RenderComponent* renderComponent;
	
	Cell2D() : renderComponent(nullptr) {};

	Cell2D(RenderComponent* _render) : renderComponent(_render) {};
};



