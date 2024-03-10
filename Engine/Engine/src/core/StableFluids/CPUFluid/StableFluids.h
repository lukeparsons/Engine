#pragma once
#include "GridStructures.h"
#include <map>
#include <vector>
#include <memory>
#include "../../../renderer/shaders/LineShader.h"
#include "../Fluid.h"

class StableFluids : public Fluid
{
private:

	std::array<float, 6> vertices = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

	GLuint VAO, VBO, textureID;

public:

	GridStructure<float> uVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> vVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> wVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> prevUVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> prevVVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> prevWVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> smoke = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> prevSmoke = GridStructure<float>(0.0f, column, row, depth);

	std::shared_ptr<LineShader> lineShader;

	StableFluids(unsigned int column, unsigned int row, unsigned int depth, int iterations = 20) : Fluid(column, row, depth, iterations) {};

	float* GetSmokeData()
	{
		return smoke.grid.data();
	}

	void Simulate(float timeStep, bool& addForceU, bool& addForceV, bool& addForceW, bool& negaddForceU, bool& negaddForceV, bool& negaddForceW, bool& addSmoke, bool& clear);
	void add_source(GridStructure<float>& grid, GridStructure<float>& prevGrid, float timeStep);
	void density_step(GridStructure<float>* grid, GridStructure<float>* prevGrid, float timeStep);
	void velocity_step(GridStructure<float>* u, GridStructure<float>* v, GridStructure<float>* w, GridStructure<float>* u0, GridStructure<float>* v0, GridStructure<float>* w0, float timeStep);
	void advect(int b, GridStructure<float>& grid, GridStructure<float>& prevGrid, GridStructure<float>& u, GridStructure<float>& v, GridStructure<float>& w, float timeStep);
	void diffuse(int b, GridStructure<float>& grid, GridStructure<float>& prevGrid, float timeStep);
	void lin_solve(int b, GridStructure<float>& grid, GridStructure<float>& prevGrid, float a, float c);
	void project(GridStructure<float>& u, GridStructure<float>& v, GridStructure<float>& w, GridStructure<float>& p, GridStructure<float>& div);
	void set_boundary(int b, GridStructure<float>& grid);

	void Profile(float timeStep, float addForceU, float addForceV, float addForceW, float negAddForceU, float negAddForceV, float negAddForceW, float addSmoke);

	void InitVelocityRender();
	void VelocityRender(Matrix4f& cameraMatrix);



};