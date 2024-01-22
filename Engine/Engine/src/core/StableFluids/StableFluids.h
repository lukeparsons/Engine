#pragma once
#include "GridStructures.h"
#include <map>
#include <vector>
#include <memory>
#include "../../scene/Scene.h"
#include "../../types/Maybe.h"
#include "../../renderer/Line.h"

static const std::shared_ptr<Texture> fluidTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/smoke.png"));
static const std::shared_ptr<Texture> solidTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/block.png"));

class StableFluids
{
private:
	EntityID fluidID;

	unsigned int column, row, depth, N;

	std::shared_ptr<Mesh> cellMesh = std::make_shared<Mesh>("../Engine/assets/box.obj");

	float viscosity;

	std::array<float, 6> vertices = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

	GLuint VAO, VBO, textureID;

public:
	const float cellWidth;
	float density;

	GridStructure<float> uVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> vVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> wVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> prevUVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> prevVVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> prevWVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> smoke = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> prevSmoke = GridStructure<float>(0.0f, column, row, depth);

	GridStructure<GridDataPoint> gridData = GridStructure<GridDataPoint>(GridDataPoint(GridDataPoint::FLUID), column, row, depth);

	std::shared_ptr<LineShader> lineShader;
	std::shared_ptr<BasicShader> meshShader;

	// TODO: include location
	StableFluids(unsigned int _row, unsigned int _column, unsigned int _depth, const Vector3f location, float _viscosity, float _cellWidth)
		: row(_row), column(_column), depth(_depth), viscosity(_viscosity), cellWidth(_cellWidth), N(std::max(std::max(column, row), depth))
	{}

	void Simulate(float timeStep, float diffRate, bool& addForceU, bool& addForceV, bool& addForceW, bool& negaddForceU, bool& negaddForceV, bool& negaddForceW, bool& addSmoke);
	void add_source(GridStructure<float>& grid, GridStructure<float>& prevGrid, float timeStep);
	void density_step(GridStructure<float>* grid, GridStructure<float>* prevGrid, float diffRate, float timeStep);
	void velocity_step(GridStructure<float>* u, GridStructure<float>* v, GridStructure<float>* w, GridStructure<float>* u0, GridStructure<float>* v0, GridStructure<float>* w0, float timeStep);
	void advect(int b, GridStructure<float>& grid, GridStructure<float>& prevGrid, GridStructure<float>& u, GridStructure<float>& v, GridStructure<float>& w, float timeStep);
	void diffuse(int b, GridStructure<float>& grid, GridStructure<float>& prevGrid, float diffRate, float timeStep);
	void lin_solve(int b, GridStructure<float>& grid, GridStructure<float>& prevGrid, float a, float c);
	void project(GridStructure<float>& u, GridStructure<float>& v, GridStructure<float>& w, GridStructure<float>& p, GridStructure<float>& div);
	void set_boundary(int b, GridStructure<float>& grid);

	void InitVelocityRender();
	void VelocityRender(Matrix4f& cameraMatrix);
	void InitModelRender();
	void ModelRender(Matrix4f& cameraMatrix);
	
	inline float colourClamp(float val)
	{
		return (std::max(0.0f, val) * 0.01f);
	}

};