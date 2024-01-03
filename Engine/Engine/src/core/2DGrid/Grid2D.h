#pragma once
#include "GridStructures2D.h"
#include "RowVector2D.h"
#include "../../scene/components/RenderComponent.h"
#include <map>
#include "../../scene/Scene.h"

static const std::shared_ptr<Texture> fluidTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/smoke.png"));
static const std::shared_ptr<Texture> solidTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/block.png"));
static const std::shared_ptr<Texture> emptyTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/wall2.png"));

class Grid2D
{
private:
	EntityID fluidID;
	RenderComponent* fluidRenderComponent;

	unsigned int column, row;

	TextureData<unsigned char> gridTexture;

	const float scaleCellWidth;
	void clamp_to_grid(float x, float y, unsigned int& i, unsigned int& j);
	void calculate_initial_distances();
	void extrapolate();

	GridStructure2DHalo<float> signedDistance = GridStructure2DHalo<float>(0, column, row);

public:
	const float cellWidth;
	float density;

	GridStructure2DHalo<float> uVelocity = GridStructure2DHalo<float>(0, column, row);
	GridStructure2DHalo<float> vVelocity = GridStructure2DHalo<float>(0, column, row);
	GridStructure2DHalo<float> pressure = GridStructure2DHalo<float>(0, column, row);
	GridStructure2DHalo<float> smoke = GridStructure2DHalo<float>(0, column, row);
	GridStructure2DHalo<float> temparature = GridStructure2DHalo<float>(0, column, row);

	GridStructure2DHalo<GridDataPoint2D> gridData = GridStructure2DHalo<GridDataPoint2D>(GridDataPoint2D(GridDataPoint2D::EMPTY), column, row);

	RowVector2D negativeDivergences = RowVector2D(column, row);
	RowVector2D precon = RowVector2D(column, row);

	Grid2D(unsigned int _row, unsigned int _column, Scene& scene, std::shared_ptr<Mesh>& gridModel, const Vector2f& location, float _density, float _cellWidth)
		: row(_row), column(_column), density(_density), cellWidth(_cellWidth), scaleCellWidth(1 / _cellWidth),
		gridTexture(TextureData<unsigned char>(column, row, GL_RGBA8, GL_UNSIGNED_BYTE, std::vector<unsigned char>()))
	{

		fluidID = scene.CreateModel(gridModel, solidTexture, Vector3f(0, 0, 0), Vector3f(1, 1, 1));
		fluidRenderComponent = scene.GetComponent<RenderComponent>(fluidID);

		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				gridData(i, j).cellState = GridDataPoint2D::FLUID;
				gridTexture.pixels.push_back(0);
				gridTexture.pixels.push_back(0);
				gridTexture.pixels.push_back(255);
				gridTexture.pixels.push_back(255);
			}
		}

		uVelocity.initLeftHalo(6.0f);
		smoke.initLeftHalo(100.0f);
		pressure.initLeftHalo(20.0f);
		//vVelocity.initLeftHalo(9.81f);

		// Make the boundary solid
		for(unsigned int i = 0; i < column; i++)
		{
			gridData(i, 0).cellState = GridDataPoint2D::SOLID;
			gridData(i, row - 1).cellState = GridDataPoint2D::SOLID;
		}

		for(unsigned int j = 0; j < row; j++)
		{
			//gridData(0, j).cellState = GridDataPoint::SOLID;
			gridData(column - 1, j).cellState = GridDataPoint2D::SOLID;
		}

		calculate_initial_distances();
	}

	void advect(float timeStep);

	void PCGSolve(float timeStep);
	void PCG();
	void applyA(const RowVector2D& vector, RowVector2D& result);
	void applyPreconditioner(RowVector2D& residualVector, RowVector2D& auxiliaryVector);
	void constructPreconditioner();

	void GaussSeidel(float timeStep);

	void boundaryConditions()
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
	}

	void addgravity(float timeStep)
	{
		for(unsigned int i = 0; i < row; i++)
		{
			for(unsigned int j = 0; j < column; j++)
			{
				if(gridData(i, j).cellState == GridDataPoint2D::FLUID)
				{
					vVelocity(i, j) += timeStep * -9.81f;
				}
			}
		}
	}

	void project(float timeStep)
	{
		//boundaryConditions();
		PCGSolve(timeStep);
	}

	inline unsigned char colourClamp(float val)
	{
		return static_cast<unsigned char>(std::max(0.0f, val) * 12.75f);
	}

	void UpdateTexture()
	{
		unsigned int offset = 0;
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				if(gridData(i, j).cellState == GridDataPoint2D::FLUID)
				{
					gridTexture.pixels[offset] = 0;
					gridTexture.pixels[1 + offset] = 0;
					gridTexture.pixels[2 + offset] = 0;
					gridTexture.pixels[3 + offset] = colourClamp(smoke(i, j));

				}

				offset += 4;
			}
		}
		fluidRenderComponent->ChangeTextureData(gridTexture.pixels);
	}

	void PrintCell(unsigned int i, unsigned int j)
	{
		std::cout << i << ", " << j << std::endl;
		std::cout << "uVelocity " << uVelocity(i, j) << std::endl;
		std::cout << "vVelocity " << vVelocity(i, j) << std::endl;
		std::cout << "Pressure " << pressure(i, j) << std::endl;
		std::cout << std::endl; 
	}
};