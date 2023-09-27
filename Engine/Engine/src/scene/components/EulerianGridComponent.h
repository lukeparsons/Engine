#pragma once
#include <glad/glad.h>
#include "../../types/EngineTypes.h"
#include "Component.h"
#include "../../renderer/Mesh.h"
#include "../../math/Matrix4f.h"

class Scene;

class Cell
{
public:
	EntityID id;
	std::shared_ptr<Mesh> mesh;
	GLuint colourLoc;

	Cell(EntityID _id, std::shared_ptr<Mesh> _mesh) : id(_id), mesh(_mesh) 
	{
		colourLoc = glGetUniformLocation(mesh->shaderProgram->GetID(), "FluidColour");
	};
};

class EulerianGridComponent : public Component
{
private:
	size_t row, column, depth;

public:
	std::vector<Cell> cells;

	EulerianGridComponent(EntityID id) : Component(id), row(0), column(0), depth(0) {};

	void InitializeGrid(Scene& scene, Mesh& gridModel, size_t _row, size_t _column, size_t depth, const Vector3f& location);
};