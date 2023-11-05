#pragma once
#include "../../renderer/Mesh.h"
#include "../../scene/Scene.h"
#include "../../scene/components/RenderComponent.h"
#include <map>
#include "../../math/Matrix.h"

class Cell2D
{
private:
	GLuint shaderID;
public:
	RenderComponent* renderComponent;

	inline GLuint GetShaderID()
	{
		return shaderID;
	}

	Cell2D() : renderComponent(nullptr), shaderID(0) {};
	Cell2D(RenderComponent* _render) : renderComponent(_render), shaderID(_render->mesh->shaderProgram->GetID()) {};
};



