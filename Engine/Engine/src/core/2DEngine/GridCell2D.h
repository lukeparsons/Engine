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
	enum State { FLUID, SOLID, EMPTY } cellState;
	
	Cell2D() : cellState(SOLID), renderComponent(nullptr) {};

	Cell2D(RenderComponent* _render) : renderComponent(_render), cellState(SOLID) {};
};



