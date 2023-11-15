#pragma once
#include "GridCell2D.h"
#include "GridStructures.h"
#include "RowVector.h"
#include "../../scene/components/RenderComponent.h"
#include <map>

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

	bool snap_to_grid(unsigned int& i, unsigned int& j)
	{
		bool snapped = false;
		if(i < 0) // Outside grid on left
		{
			i = 0;
			snapped = true;
		} else if(i >= column) // Outside grid on right
		{
			i = column - 1;
			snapped = true;
		}

		if(j < 0) // Outside grid below
		{
			j = 0;
			return true;
		} else if(j >= row) // Outside grid above
		{
			j = row - 1;
			return true;
		}
		return snapped;
	}


	inline float calculate_interp_quantity_i(int i, int j, std::array<float, 4> weights, const GridStructureHalo<float>& data) const
	{
		return weights[0] * data(i, j - 1) + weights[1] * data(i, j) + weights[2] * data(i, j + 1) + weights[3] * data(i, j + 2);
	}

	inline float calculate_interp_quantity_j(int i, int j, std::array<float, 4> weights, const GridStructureHalo<float>& data) const
	{
		return weights[0] * data(i - 1, j) + weights[1] * data(i, j) + weights[2] * data(i + 1, j) + weights[3] * data(i + 2, j);
	}

public:
	// TODO: make private
	const float cellWidth;
	float density;
	float temperature;
	float concentration;

	GridStructureHalo<float> uVelocity = GridStructureHalo<float>(0, column, row);
	GridStructureHalo<float> vVelocity = GridStructureHalo<float>(0, column, row);
	GridStructureHalo<float> pressure = GridStructureHalo<float>(0, column, row);
	GridStructureHalo<float> smoke = GridStructureHalo<float>(0, column, row);

	// TODO: Make this a one thick halo
	GridStructureHalo<GridDataPoint> gridData = GridStructureHalo<GridDataPoint>(GridDataPoint(GridDataPoint::FLUID), column, row);

	RowVector negativeDivergences = RowVector(column, row);
	RowVector precon = RowVector(column, row);

	Grid2D(unsigned int _row, unsigned int _column, Scene& scene, std::shared_ptr<Mesh>& gridModel, const Vector2f& location, float _density, float _cellWidth)
		: row(_row), column(_column), density(_density), cellWidth(_cellWidth), gridTexture(TextureData<unsigned char>(column, row, GL_RGBA8, GL_UNSIGNED_BYTE, std::vector<unsigned char>()))
	{
		Vector3f cellScale = Vector3f(cellWidth, cellWidth, cellWidth);

		fluidID = scene.CreateModel(gridModel, solidTexture, Vector3f(0, 0, 0), Vector3f(1, 1, 1));
		fluidRenderComponent = scene.GetComponent<RenderComponent>(fluidID);

		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{

				if(i >= 0.3 * column && i <= 0.5 * column)
				{
					if(j >= 0.4 * row && j <= 0.6 * row)
					{
						//gridData(i, j).cellState = GridDataPoint::SOLID;
					}
				}

				if(i == 1)
				{
					uVelocity(1, j) = 11.0f;
					smoke(1, j) = 100.0f;
				}

				//EntityID cellID = scene.CreateModel(gridModel, fluidTexture,
					//Vector3f(location.x, location.y, 0) + Vector3f(i * cellWidth * 2, j * cellWidth * 2, 0), cellScale);
				//RenderComponent* render = scene.GetComponent<RenderComponent>(cellID);
				
				gridTexture.pixels.push_back(0);
				gridTexture.pixels.push_back(0);
				gridTexture.pixels.push_back(255);
				gridTexture.pixels.push_back(255);
			}
		}

		// Make the boundary solid
		for(unsigned int i = 0; i < column; i++)
		{
			gridData(i, 0).cellState = GridDataPoint::SOLID;
			gridData(i, row - 1).cellState = GridDataPoint::SOLID;

		}

		for(unsigned int j = 1; j < row - 1; j++)
		{
			gridData(0, j).cellState = GridDataPoint::SOLID;
			gridData(column - 1, j).cellState = GridDataPoint::SOLID;
		}

	}

	void advect(float timeStep, GridStructureHalo<float>& data1, GridStructureHalo<float>& data2);

	// PCG (TODO: move all but PCGSolve out of function)
	void PCGSolve(float timeStep);
	void PCG();
	void applyA(const RowVector& vector, RowVector& result);
	void applyPreconditioner(RowVector& residualVector, RowVector& auxiliaryVector);
	void constructPreconditioner();
	void UpdateA(float Acoefficient, unsigned int i, unsigned int j);

	// GaussSeidel
	void GaussSeidel(float timeStep);

	void addforces(float timeStep, float force)
	{
		for(unsigned int i = 0; i < row; i++)
		{
			for(unsigned int j = 0; j < column; j++)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					//uVelocity(i, j) += timeStep * force;
					vVelocity(i, j) += timeStep * force;
				}
			}
		}

		// Temp
		for(unsigned int j = 1; j < row - 1; j++)
		{
			if(gridData(1, j).cellState == GridDataPoint::FLUID)
			{
				//uVelocity(1, j) += 2.0f * timeStep;
				//smoke(1, j) += 2.0f;
			}
		}
	}

	void Solve(float timeStep)
	{
		GaussSeidel(timeStep);

		//temp extrapolate
		for(unsigned int i = 1; i < column - 1; i++)
		{
			uVelocity(i, 1) = uVelocity(i, 2);
			uVelocity(i, row - 2) = uVelocity(i, row - 3);
		}

		for(unsigned int j = 1; j < row - 1; j++)
		{
			vVelocity(1, j) = vVelocity(2, j);
			vVelocity(column - 2, j) = vVelocity(column - 3, j);
		}
	}


	inline unsigned char colourClamp(float maxVal, float minVal, float val)
	{
		return static_cast<unsigned char>(std::max(std::min(val, maxVal), minVal) * 255);
	}

	void UpdateTexture()
	{
		float smokeMax = smoke.max();
		float smokeMin = smoke.min();
		unsigned int offset = 0;
		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					gridTexture.pixels[offset] = 0;
					gridTexture.pixels[1 + offset] = 0;
					gridTexture.pixels[2 + offset] = 0;
					if(smoke(i, j) > 0)
					{
						gridTexture.pixels[3 + offset] = colourClamp(smokeMax, smokeMin, smoke(i, j));
					} else
					{
						gridTexture.pixels[3 + offset] = 0;
					}

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