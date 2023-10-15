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
	enum State { FLUID, SOLID, EMPTY, DEFAULT } cellState;
	
	Cell2D() : cellState(DEFAULT), renderComponent(nullptr) {};

	Cell2D(State _cellState, RenderComponent* _render) : renderComponent(_render), cellState(_cellState) {};
};



