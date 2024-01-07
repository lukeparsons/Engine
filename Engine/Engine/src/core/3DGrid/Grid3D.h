#pragma once
#include "GridStructures.h"
#include "RowVector.h"
#include <map>
#include <vector>
#include "../../scene/Scene.h"
#include "../../types/Maybe.h"

struct GridPoint
{
	unsigned int x, y, z;
	GridPoint(unsigned int _x, unsigned int _y, unsigned int _z) : x(_x), y(_y), z(_z) {};
};

static const std::shared_ptr<Texture> fluidTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/smoke.png"));
static const std::shared_ptr<Texture> solidTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/block.png"));

class Grid3D
{
private:
	EntityID fluidID;
	RenderComponent* fluidRenderComponent;

	unsigned int column, row, depth;

	const float scaleCellWidth;
	void clamp_to_grid(float x, float y, float z, unsigned int& i, unsigned int& j, unsigned int& k);
	void calculate_initial_distances();
	void extrapolate();

	GridStructureHalo<float> signedDistance = GridStructureHalo<float>(std::numeric_limits<float>::max(), column, row, depth);
	GridStructureHalo<Maybe<GridPoint>> closestPoint = GridStructureHalo<Maybe<GridPoint>>(Nothing(), column, row, depth); // TODO: Make this one thick halo only!

	std::shared_ptr<Mesh> cellMesh = std::make_shared<Mesh>("../Engine/assets/box.obj");

public:
	const float cellWidth;
	float density;

	UVelocityGrid uVelocity = UVelocityGrid(0.0f, column, row, depth);
	VVelocityGrid vVelocity = VVelocityGrid(0.0f, column, row, depth);
	WVelocityGrid wVelocity = WVelocityGrid(0.0f, column, row, depth);
	GridStructureHalo<float> pressure = GridStructureHalo<float>(0.0f, column, row, depth);
	GridStructureHalo<float> smoke = GridStructureHalo<float>(0.0f, column, row, depth);
	GridStructureHalo<float> temparature = GridStructureHalo<float>(0, column, row, depth);

	GridStructureHalo<GridDataPoint> gridData = GridStructureHalo<GridDataPoint>(GridDataPoint(GridDataPoint::EMPTY), column, row, depth);

	RowVector negativeDivergences = RowVector(column, row, depth);
	RowVector precon = RowVector(column, row, depth);

	// TODO: include location
	Grid3D(unsigned int _row, unsigned int _column, unsigned int _depth, Scene& scene, const Vector3f location, float _density, float _cellWidth)
		: row(_row), column(_column), depth(_depth), density(_density), cellWidth(_cellWidth), scaleCellWidth(1 / _cellWidth)
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

		/*uVelocity.initBottomHalo(6.0f);
		smoke.initBottomHalo(100.0f);
		pressure.initBottomHalo(20.0f);
		vVelocity.initBottomHalo(9.81f);
		wVelocity.initBottomHalo(5.0f);

		uVelocity.fillCentre(0.f);
		vVelocity.fillCentre(0.f);
		wVelocity.fillCentre(0.f);
		smoke.fillCentre(0.f);
		pressure.fillCentre(0.f); */

		// Make boundary solid
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				gridData(i, j, 0).cellState = GridDataPoint::SOLID; // front face
				//gridData(i, j, depth - 1).cellState = GridDataPoint::EMPTY; // back face
			}
		}

		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int k = 0; k < depth; k++)
			{
				//gridData(i, 0, k).cellState = GridDataPoint::SOLID; // bottom face
				uVelocity(i, 0, k) = 6.0f;
				vVelocity(i, 0, k) = 10.0f;
				pressure(i, 0, k) = 5.0f;
				smoke(i, 0, k) = 10.0f;
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

		calculate_initial_distances();
	}

	void advect(float timeStep);

	void PCGSolve(float timeStep);
	void PCG();
	void applyA(const RowVector& vector, RowVector& result);
	void applyPreconditioner(RowVector& residualVector, RowVector& auxiliaryVector);
	void constructPreconditioner();

	void GaussSeidel(float timeStep);

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
		PCGSolve(timeStep);
		//GaussSeidel(timeStep);
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


	void PrintCell(unsigned int i, unsigned int j, unsigned int k)
	{
		std::cout << i << ", " << j << std::endl;
		std::cout << "uVelocity " << uVelocity(i, j, k) << std::endl;
		std::cout << "vVelocity " << vVelocity(i, j, k) << std::endl;
		std::cout << "Pressure " << pressure(i, j, k) << std::endl;
		std::cout << std::endl;
	}
};