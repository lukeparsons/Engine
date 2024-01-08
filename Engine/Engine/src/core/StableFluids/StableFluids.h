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
	RenderComponent* fluidRenderComponent;

	unsigned int column, row, depth, N;

	std::shared_ptr<Mesh> cellMesh = std::make_shared<Mesh>("../Engine/assets/box.obj");

public:
	const float cellWidth;
	float density;

	GridStructureHalo<float> uVelocity = GridStructureHalo<float>(0.0f, column, row, depth);
	GridStructureHalo<float> vVelocity = GridStructureHalo<float>(0.0f, column, row, depth);
	GridStructureHalo<float> wVelocity = GridStructureHalo<float>(0.0f, column, row, depth);
	GridStructureHalo<float> pressure = GridStructureHalo<float>(0.0f, column, row, depth);
	GridStructureHalo<float> smoke = GridStructureHalo<float>(0.0f, column, row, depth);

	GridStructureHalo<GridDataPoint> gridData = GridStructureHalo<GridDataPoint>(GridDataPoint(GridDataPoint::EMPTY), column, row, depth);

	// TODO: include location
	StableFluids(unsigned int _row, unsigned int _column, unsigned int _depth, Scene& scene, const Vector3f location, float _density, float _cellWidth)
		: row(_row), column(_column), depth(_depth), density(_density), cellWidth(_cellWidth), N(_row * _column * _depth)
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
	}

	void Simulate(float timeStep);
	void advect(float timeStep)
	void diffuse(float timeStep, float diff, GridStructureHalo<float>& grid, GridStructureHalo<float>& prevGrid);
	void project(float timeStep)

	void UpdateRender()
	{
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
						case GridDataPoint::EMPTY:
							if(smoke(i, j, k) > 1.0f)
							{
								//gridData(i, j, k).cellState = GridDataPoint::FLUID;
								gridData(i, j, k).render->isActive = true;
							} else
							{
								//gridData(i, j, k).cellState = GridDataPoint::EMPTY;
								gridData(i, j, k).render->isActive = false;
							}
					}
				}
			}
		}
	}
};