#pragma once
#include "GridStructures.h"
#include <map>
#include <vector>
#include "../../scene/Scene.h"
#include "../../types/Maybe.h"

static const std::shared_ptr<Texture> fluidTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/smoke.png"));
static const std::shared_ptr<Texture> solidTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/block.png"));

class StableFluids
{
private:
	EntityID fluidID;

	unsigned int column, row, depth, N;

	std::shared_ptr<Mesh> cellMesh = std::make_shared<Mesh>("../Engine/assets/box.obj");

	float viscosity;

public:
	const float cellWidth;
	float density;

	GridStructure<float> uVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> prevUVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> prevVVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> vVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> wVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> prevWVelocity = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> smoke = GridStructure<float>(0.0f, column, row, depth);
	GridStructure<float> prevSmoke = GridStructure<float>(0.0f, column, row, depth);

	GridStructure<GridDataPoint> gridData = GridStructure<GridDataPoint>(GridDataPoint(GridDataPoint::FLUID), column, row, depth);

	// TODO: include location
	StableFluids(unsigned int _row, unsigned int _column, unsigned int _depth, Scene& scene, const Vector3f location, float _viscosity, float _cellWidth)
		: row(_row), column(_column), depth(_depth),viscosity(_viscosity), cellWidth(_cellWidth), N(std::max(std::max(column, row), depth))
	{
		float scale1 = 0.01 * 5;
		float scale2 = 0.01 * 10;
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				for(unsigned int k = 0; k < depth; k++)
				{
					gridData(i, j, k).cellState = GridDataPoint::FLUID;
					EntityID id = scene.CreateModel(cellMesh, fluidTexture, Vector3f(i * scale2, j * scale2, k * scale2), Vector3f(scale1, scale1, scale1));
					gridData(i, j, k).render = scene.GetComponent<RenderComponent>(id);

				}
			}
		}
		smoke(column / 2, row / 2, depth - 1) = 200.f;
		uVelocity(column / 2, row / 2, depth - 1) = 100.f;
		//uVelocity.fill(100.f);
		vVelocity.fill(0.01f);
		//wVelocity.fill(0.2f);
	}

	void Simulate(float timeStep, float diffRate);
	void add_source(float timeStep, GridStructure<float>& grid, GridStructure<float>& prevGrid);
	void density_step(float timeStep, float diffRate, GridStructure<float>* density, GridStructure<float>* prevDensity);
	void velocity_step(float timeStep, GridStructure<float>* uVel, GridStructure<float>* vVel, GridStructure<float>* wVel,
		GridStructure<float>* prevUVel, GridStructure<float>* prevVVel, GridStructure<float>* prevWVelp);
	void advect(float timeStep, GridStructure<float>& grid, GridStructure<float>& prevGrid, GridStructure<float>& uVel, GridStructure<float>& vVel, GridStructure<float>& wVel, unsigned int b);
	void diffuse(float timeStep, float diff, GridStructure<float>& grid, GridStructure<float>& prevGrid, unsigned int b);
	void lin_solve(float scale, GridStructure<float>& grid, GridStructure<float>& prevGrid, unsigned int b, float c);
	void project(float timeStep, GridStructure<float>& uVel, GridStructure<float>& vVel, GridStructure<float>& wVel, GridStructure<float>& div, GridStructure<float>& p);
	void set_boundary(GridStructure<float>& grid, unsigned int b);
	
	inline float colourClamp(float val)
	{
		return (std::max(0.0f, val) * 0.01f);
	}

	void UpdateRender()
	{
		//std::cout << colourClamp(uVelocity(column / 2, row / 2, depth - 1)) << ", " << uVelocity(column / 2, row / 2, depth - 1) << std::endl;
		for(unsigned int i = 0; i < row; i++)
		{
			for(unsigned int j = 0; j < column; j++)
			{
				for(unsigned int k = 0; k < column; k++)
				{
					switch(gridData(i, j, k).cellState)
					{
						case GridDataPoint::SOLID:
							gridData(i, j, k).render->ChangeTexture(solidTexture);
							break;
						case GridDataPoint::FLUID:
							if(uVelocity(i, j, k) < 0.1f)
							{
								gridData(i, j, k).render->isActive = true;
								gridData(i, j, k).render->SetColour({ colourClamp(uVelocity(i, j, k)), colourClamp(vVelocity(i, j, k)), colourClamp(wVelocity(i, j, k)) });
							} else
							{
								gridData(i, j, k).render->isActive = false;
							}
					}
				}
			}
		}
	}
};