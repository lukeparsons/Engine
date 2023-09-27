#pragma once
#include "../../renderer/Mesh.h"
#include "../../scene/Scene.h"
#include "../../scene/components/RenderComponent.h"
#include <map>
#include "../../math/Matrix.h"

using IntPair = std::pair<int, int>;

class Cell2D
{
public:
	RenderComponent* renderComponent;
	enum State { FLUID, SOLID, EMPTY } cellState;
	
	Cell2D() : cellState(SOLID), renderComponent(nullptr) {};

	Cell2D(RenderComponent* _render) : renderComponent(_render), cellState(SOLID) {};
};

class BorderCell2D : public Cell2D
{
public:
	enum BorderType { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, TOP, BOTTOM, LEFT, RIGHT } borderType;

	BorderCell2D() : Cell2D(), borderType(TOP_LEFT) {};

	BorderCell2D(RenderComponent* _render, BorderType _borderType) : Cell2D(_render), borderType(_borderType) {};
};



