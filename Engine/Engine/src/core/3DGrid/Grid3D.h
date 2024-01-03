#pragma once
#include "GridStructures.h"
#include "RowVector.h"
#include <map>
#include <vector>
#include "../../scene/Scene.h"

static const std::shared_ptr<Texture> fluidTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/smoke.png"));
static const std::shared_ptr<Texture> solidTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/block.png"));
static const std::shared_ptr<Texture> emptyTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/wall2.png"));

class Grid3D
{
private:
	EntityID fluidID;
	RenderComponent* fluidRenderComponent;

	unsigned int column, row, depth;

	const float scaleCellWidth;
	void clamp_to_grid(float x, float y, float z, unsigned int& i, unsigned int& j, unsigned int& k);
	//void calculate_initial_distances();
	//void extrapolate();

	GridStructureHalo<float> signedDistance = GridStructureHalo<float>(0, column, row, depth);

	std::shared_ptr<Mesh> cellMesh = std::make_shared<Mesh>("../Engine/assets/box.obj");

public:
	const float cellWidth;
	float density;

	GridStructureHalo<float> uVelocity = GridStructureHalo<float>(0, column, row, depth);
	GridStructureHalo<float> vVelocity = GridStructureHalo<float>(0, column, row, depth);
	GridStructureHalo<float> wVelocity = GridStructureHalo<float>(0, column, row, depth);
	GridStructureHalo<float> pressure = GridStructureHalo<float>(0, column, row, depth);
	GridStructureHalo<float> smoke = GridStructureHalo<float>(0, column, row, depth);
	GridStructureHalo<float> temparature = GridStructureHalo<float>(0, column, row, depth);

	GridStructureHalo<GridDataPoint> gridData = GridStructureHalo<GridDataPoint>(GridDataPoint(GridDataPoint::EMPTY), column, row, depth);

	RowVector negativeDivergences = RowVector(column, row, depth);
	RowVector precon = RowVector(column, row, depth);

	Grid3D(unsigned int _row, unsigned int _column, unsigned int _depth, Scene& scene, const Vector2f& location, float _density, float _cellWidth)
		: row(_row), column(_column), depth(_depth), density(_density), cellWidth(_cellWidth), scaleCellWidth(1 / _cellWidth)
	{
		float scale1 = cellWidth * 5;
		float scale2 = cellWidth * 10;
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				for(unsigned int k = 0; k < depth; k++)
				{
					gridData(i, j, k).cellState = GridDataPoint::FLUID;
					EntityID id = scene.CreateModel(cellMesh, solidTexture, Vector3f(i * scale2, j * scale2, k * scale2), Vector3f(scale1, scale1, scale1));
					gridData(i, j, k).render = scene.GetComponent<RenderComponent>(id);
				}
			}
		}

		uVelocity.initLeftHalo(6.0f);
		smoke.initLeftHalo(100.0f);
		pressure.initLeftHalo(20.0f);
		//vVelocity.initLeftHalo(9.81f);

		// Make boundary solid
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				gridData(i, j, 0).cellState = GridDataPoint::FLUID; // front face
				gridData(i, j, depth - 1).cellState = GridDataPoint::SOLID; // back face
			}
		}

		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				gridData(i, 0, k).cellState = GridDataPoint::SOLID; // bottom face
				gridData(i, row - 1, k).cellState = GridDataPoint::SOLID; // top face
			}
		}

		for(unsigned int j = 0; j < row; j++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				gridData(0, j, k).cellState = GridDataPoint::SOLID; // left face
				gridData(column - 1, j, k).cellState = GridDataPoint::SOLID; // right face
			}
		}

		//calculate_initial_distances();
	}

	void advect(float timeStep);

	/*void PCGSolve(float timeStep);
	void PCG();
	void applyA(const RowVector& vector, RowVector& result);
	void applyPreconditioner(RowVector& residualVector, RowVector& auxiliaryVector);
	void constructPreconditioner();

	void GaussSeidel(float timeStep); */

	/*void boundaryConditions()
	{
		// Make the boundary solid
		for(unsigned int i = 0; i < column; i++)
		{
			//uVelocity(i, 0) = 0;
			//vVelocity(i, 0) = 0;
			uVelocity(i, row - 1) = 0;
			vVelocity(i, row - 1) = 0;
		}

		for(unsigned int j = 0; j < row; j++)
		{
			uVelocity(0, j) = 0;
			vVelocity(0, j) = 0;
			uVelocity(column - 1, j) = 0;
			vVelocity(column - 1, j) = 0;
		}
	} */

	void addgravity(float timeStep)
	{
		for(unsigned int i = 0; i < row; i++)
		{
			for(unsigned int j = 0; j < column; j++)
			{
				for(unsigned int k = 0; k < column; k++)
				{
					if(gridData(i, j, k).cellState == GridDataPoint::FLUID)
					{
						vVelocity(i, j, k) += timeStep * -9.81f;
					}
				}
			}
		}
	}

	void project(float timeStep)
	{
		//boundaryConditions();
		//PCGSolve(timeStep);
	}

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
							gridData(i, j, k).render->ChangeTexture(fluidTexture);
							break;
						case GridDataPoint::EMPTY:
							gridData(i, j, k).render->ChangeTexture(emptyTexture);
							break;
					}
				}
			}
		}
	}


	void PrintCell(unsigned int i, unsigned int j, unsigned int k)
	{
		std::cout << i << ", " << j << std::endl;
		std::cout << "uVelocity " << uVelocity(i, j, k) << std::endl;
		std::cout << "vVelocity " << vVelocity(i, j, k) << std::endl;
		std::cout << "Pressure " << pressure(i, j, k) << std::endl;
		std::cout << std::endl;
	}
};