#pragma once
#include "GridCell2D.h"
#include "GridStructures.h"
#include "RowVector.h"
#include "../../scene/components/RenderComponent.h"
#include <map>

static const std::shared_ptr<Texture> fluidTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/smoke.png"));
static const std::shared_ptr<Texture> solidTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/block.png"));
static const std::shared_ptr<Texture> emptyTexture = std::make_shared<TextureData<unsigned char>>(LoadPng("../Engine/assets/wall2.png"));

template<size_t row, size_t column>
class Grid2D
{
private:
	const float cellWidth;
	EntityID fluidID;
	RenderComponent* fluidRenderComponent;

	TextureData<float> gridTexture;

	bool snap_to_grid(int& i, int& j)
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

	inline float uvelocity_centre(size_t i, size_t j) const
	{
		return (uVelocity(i - 1, j) + uVelocity(i, j)) / 2;
	}

	inline float vvelocity_centre(size_t i, size_t j) const
	{
		return (vVelocity(i, j - 1) + vVelocity(i, j)) / 2;
	}

	std::array<float, 4> get_interp_weights(float s) const
	{
		float sSquared = pow(s, 2);
		float sCubed = pow(s, 3);

		float negativeWeight = (-1/3.0f) * s + (1/2.0f) * sSquared - (1/6.0f) * sCubed;
		float weight = 1 - sSquared + (1/2.0f) * (sCubed - s);
		float positiveWeight = s + (1/2.0f) * (sSquared - sCubed);
		float doublePositiveWeight = (1/6.0f) * (sCubed - s);

		return { negativeWeight, weight, positiveWeight, doublePositiveWeight };
	}

	template<typename T>
	inline float calculate_interp_quantity_i(int i, int j, std::array<float, 4> weights, const GridStructureHalo<T, row, column>& data) const
	{
		return weights[0] * data(i, j - 1) + weights[1] * data(i, j) + weights[2] * data(i, j + 1) + weights[3] * data(i, j + 2);
	}

	template<typename T>
	inline float calculate_interp_quantity_j(int i, int j, std::array<float, 4> weights, const GridStructureHalo<T, row, column>& data) const
	{
		return weights[0] * data(i - 1, j) + weights[1] * data(i, j) + weights[2] * data(i + 1, j) + weights[3] * data(i + 2, j);
	}

public:

	float density;
	float temperature;
	float concentration;

	GridStructureHalo<float, row, column> uVelocity = GridStructureHalo<float, row, column>(0);
	GridStructureHalo<float, row, column> vVelocity = GridStructureHalo<float, row, column>(0);
	GridStructureHalo<float, row, column> pressure = GridStructureHalo<float, row, column>(0);

	// TODO: Make this a one thick halo
	GridStructureHalo<GridDataPoint, row, column> gridData = GridStructureHalo<GridDataPoint, row, column>(GridDataPoint(GridDataPoint::FLUID));

	RowVector<row, column> negativeDivergences;
	RowVector<row, column> precon;

	Grid2D(Scene& scene, std::shared_ptr<Mesh>& gridModel, const Vector2f& location, float _density, float _cellWidth) : density(_density), cellWidth(_cellWidth),
		gridTexture(TextureData<float>(row, column, GL_RGB, GL_FLOAT, std::vector<float>()))
	{
		Vector3f cellScale = Vector3f(cellWidth, cellWidth, cellWidth);

		fluidID = scene.CreateModel(gridModel, solidTexture, Vector3f(location.x, location.y, 0), Vector3f(cellWidth * column * 0.4, cellWidth * row * 0.4, cellWidth));
		fluidRenderComponent = scene.GetComponent<RenderComponent>(fluidID);

		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				//EntityID cellID = scene.CreateModel(gridModel, fluidTexture,
					//Vector3f(location.x, location.y, 0) + Vector3f(i * cellWidth * 2, j * cellWidth * 2, 0), cellScale);
				//RenderComponent* render = scene.GetComponent<RenderComponent>(cellID);
				
				gridTexture.pixels.push_back(uVelocity(i, j));
				gridTexture.pixels.push_back(vVelocity(i, j));
				gridTexture.pixels.push_back(pressure(i, j));
			}
		}

		// Make the boundary solid
		for(unsigned int i = 0; i < column; i++)
		{
			//gridData(i, 0).cellState = GridDataPoint::SOLID;
			//gridData(i, 0).cell.renderComponent->ChangeTexture(solidTexture);
			//gridData(i, row - 1).cellState = GridDataPoint::SOLID;
			//gridData(i, row - 1).cell.renderComponent->ChangeTexture(solidTexture);
		}

		for(unsigned int j = 0; j < row; j++)
		{
			//gridData(0, j).cellState = GridDataPoint::SOLID;
			//gridData(0, j).cell.renderComponent->ChangeTexture(solidTexture);
			//gridData(column - 1, j).cellState = GridDataPoint::SOLID;
			//gridData(column - 1, j).cell.renderComponent->ChangeTexture(solidTexture);
		}

		// Temp
		for(unsigned int j = 1; j < row - 1; j++)
		{
			//uVelocity(1, j) = 2.0f;
		}
	}

	void advect(float timeStep, GridStructureHalo<float, row, column>& data)
	{
		float scale = 1 / cellWidth;

		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					// Runge-Kutta 2
					int midPointi = static_cast<int>(std::floorf(i - 0.5 * timeStep * uVelocity(i, j)));
					int midPointj = static_cast<int>(std::floorf(j - 0.5 * timeStep * vVelocity(i, j)));
					//std::cout << "midpoint (" << midPointi << ", " << midPointj << ")" << std::endl;
					if(snap_to_grid(midPointi, midPointj))
					{
						data(i, j) = data(midPointi, midPointj);
						continue;
					}

					float originalExactPointi = i - 0.5 * timeStep * uvelocity_centre(midPointi, midPointj);
					float originalExactPointj = j - 0.5 * timeStep * vvelocity_centre(midPointi, midPointj);
					//std::cout << "exact(" << originalExactPointi << ", " << originalExactPointj << ")" << std::endl;

					// Interpolate
					float alphai = (originalExactPointi - i) * scale;
					float alphaj = (originalExactPointj - j) * scale;

					std::array<float, 4> weightsi = get_interp_weights(alphai);
					std::array<float, 4> weightsj = get_interp_weights(alphaj);

					int originali = static_cast<int>(std::floorf(originalExactPointi));
					int originalj = static_cast<int>(std::floorf(originalExactPointj));

					//std::cout << "From (" << i << ", " << j << ") to (" << originali << ", " << originalj << ")" << std::endl;

					if(snap_to_grid(originali, originalj))
					{
						data(i, j) = data(midPointi, midPointj);
						continue;
					}

					//Cubic interpolation

					// x-axis
					float negativeQi = calculate_interp_quantity_i(originali - 1, originalj, weightsi, data);
					float Qi = calculate_interp_quantity_i(originali, originalj, weightsi, data);
					float positiveQi = calculate_interp_quantity_i(originali + 1, originalj, weightsi, data);
					float doublePositiveQi = calculate_interp_quantity_i(originali + 2, originalj, weightsi, data);

					float finalQi = weightsi[0] * negativeQi + weightsi[1] * Qi + weightsi[2] * positiveQi + weightsi[3] * doublePositiveQi;

					// y-axis
					float negativeQj = calculate_interp_quantity_j(originali, originalj - 1, weightsj, data);
					float Qj = calculate_interp_quantity_j(originali, originalj, weightsj, data);
					float positiveQj = calculate_interp_quantity_j(originali, originalj + 1, weightsj, data);
					float doublePositiveQj = calculate_interp_quantity_j(originali, originalj + 2, weightsj, data);

					float finalQj = weightsj[0] * negativeQj + weightsj[1] * Qj + weightsj[2] * positiveQj + weightsj[3] * doublePositiveQj;

					float averageQ = (finalQi + finalQj) / 2;
					data(i, j) = averageQ;
					//gridData(i, j).*quantity = averageQ >= 0 ? averageQ : 0;

				}

			}
		}
	}

	inline void addforces(float timeStep, float force)
	{
		for(size_t i = 0; i < row; i++)
		{
			for(size_t j = 0; j < column; j++)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					uVelocity(i, j) += timeStep * force + i; // TODO: remove + i and + j
					vVelocity(i, j) += timeStep * force + j;
				}

			}
		}

	}

	void StandardPCG()
	{
		// PCG algorithm for solving Ap = b
		pressure.fill(0); // Pressure guess
		precon.fill(0);
		constructPreconditioner();

		if(std::all_of(negativeDivergences.begin(), negativeDivergences.end(), [](float val) { return val == 0; }))
		{
			return;
		}

		RowVector<row, column> residualVector = negativeDivergences;
		RowVector<row, column> auxiliaryVector;

		applyPreconditioner(residualVector, auxiliaryVector);
		RowVector<row, column> searchVector = auxiliaryVector;

		double sigma = DotProduct(auxiliaryVector, residualVector);

		// TODO: Implement proper scaling
		double tolerance = 0.0001;

		for(unsigned int iter = 0; iter < 200; iter++) // 200 here is max iterations
		{
			applyA(searchVector, auxiliaryVector);
			double dP = DotProduct(auxiliaryVector, searchVector);
			double alpha = sigma / dP;

			for(size_t i = 0; i < column; i++)
			{
				for(size_t j = 0; j < row; j++)
				{
					pressure(i, j) += alpha * searchVector(i, j);
					residualVector(i, j) -= alpha * auxiliaryVector(i, j); 			//residualVector = residualVector - (alpha * auxiliaryVector);
				}
			}

			if(residualVector.max() <= tolerance)
			{
				std::cout << "Completed in " << iter << " iterations" << std::endl;
				return;
			}

			applyPreconditioner(residualVector, auxiliaryVector);
			double sigmaNew = DotProduct(auxiliaryVector, residualVector);
			double beta = sigmaNew / sigma;

			for(size_t i = 0; i < column; i++)
			{
				for(size_t j = 0; j < row; j++)
				{
					searchVector(i, j) = auxiliaryVector(i, j) + (beta * searchVector(i, j));
				}
			}

			sigma = sigmaNew;
		}

		// Iteration limit exceeded
		std::cout << "exceeded iterations" << std::endl;
	}

	void applyA(const RowVector<row, column>& vector, RowVector<row, column>& result)
	{
		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				result(i, j) = gridData(i, j).Adiag * vector(i, j)
					+ gridData(i, j).Ax * vector(i + 1, j)
					+ gridData(i, j).Ay * vector(i, j + 1);
			}
		}
	}

	/* Modified Incomplete Choleksy preconditoner to find a matrix M where M is approx A^ {-1}
	 Simple to implement, fairly efficient and robust in handling irregular domains (e.g liquid splash)
	 However hard to efficiently parallelize and not optimally scalable
	*/
	void applyPreconditioner(RowVector<row, column>& residualVector, RowVector<row, column>& auxiliaryVector)
	{

		double t = 0;

		RowVector<row, column> q;

		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					double prevIAx = gridData(i - 1, j).Ax;
					double prevIAy = gridData(i - 1, j).Ay;
					double prevJAx = gridData(i, j - 1).Ax;
					double prevJAy = gridData(i, j - 1).Ay;

					// Solve Lq = r
					t = residualVector(i, j) - prevIAx * precon(i - 1, j) * q(i - 1, j)
						- prevJAy * precon(i, j - 1) * q(i, j - 1);

					q(i, j) = t * precon(i, j);
				}

			}
		}

		for(size_t i = column - 1; i-- > 0;)
		{
			for(size_t j = row - 1; j-- > 0;)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					t = q(i, j) - gridData(i, j).Ax * precon(i, j) * auxiliaryVector(i + 1, j)
						- gridData(i, j).Ay * precon(i, j) * auxiliaryVector(i, j + 1);
				}

				auxiliaryVector(i, j) = t * precon(i, j);
			}
		}
	}

	void constructPreconditioner()
	{
		double tuningConstant = 0.97;
		double safetyConstant = 0.25;

		double e = 0;
		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					double prevIAx = gridData(i - 1, j).Ax;
					double prevIAy = gridData(i - 1, j).Ay;
					double prevJAx = gridData(i, j - 1).Ax;
					double prevJAy = gridData(i, j - 1).Ay;

					double Adiag = gridData(i, j).Adiag;

					e = Adiag - pow(prevIAx * precon(i - 1, j), 2)
						- pow(prevJAy * precon(i, j - 1), 2)
						- tuningConstant * (prevIAx * prevIAy * pow(precon(i - 1, j), 2)
							+ prevJAy * (prevJAx * pow(precon(i, j - 1), 2)));


					if(e < safetyConstant * Adiag)
					{
						e = Adiag;
					}

					precon(i, j) = 1 / sqrt(e);
				}
			}
		}
	}

	void UpdateA(float Acoefficient, size_t i, size_t j)
	{

		GridDataPoint& cellData = gridData(i, j);

		if(gridData(i - 1, j).cellState == GridDataPoint::FLUID) // Left neighbour
		{
			cellData.Adiag += Acoefficient;
		}

		GridDataPoint::CellState rightState = gridData(i + 1, j).cellState;
		if(rightState == GridDataPoint::FLUID) // Right neighbour
		{
			cellData.Adiag += Acoefficient;
			cellData.Ax = -Acoefficient;
		} else if(rightState == GridDataPoint::EMPTY)
		{
			cellData.Adiag += Acoefficient;
		}

		if(gridData(i, j - 1).cellState == GridDataPoint::FLUID) // Below neighbour
		{
			cellData.Adiag += Acoefficient;
		}

		GridDataPoint::CellState aboveState = gridData(i, j + 1).cellState;
		if(aboveState == GridDataPoint::FLUID) // Above neighbour
		{
			cellData.Adiag += Acoefficient;
			cellData.Ay = -Acoefficient;
		} else if(aboveState == GridDataPoint::EMPTY)
		{
			cellData.Adiag += Acoefficient;
		}

	}

	// TODO: A 1xn or nx1 grid is currently broken
	void Solve(float timeStep)
	{
		float divergenceScale = 1.0f / cellWidth;
		float Acoefficient = timeStep / (density * cellWidth * cellWidth);

		size_t i = 0;
		size_t j = 0;

		/* uVelocity in the GridDataPoint refers to the right u velocity arrow for that cell in the MAC Grid
		* Likewise vVelocity refers to the up v velocity arrow for that cell
		*/

		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{

				switch(gridData(i, j).cellState)
				{
					case GridDataPoint::SOLID:
						// TODO: Remember to update Adiag, Ax etc in a ChangeState function
						/*if(gridData(i - 1, j).cellState == FLUID)
						{
							// pg 71
						} */

						break;
					case GridDataPoint::FLUID:

						// Pressure coefficient update

						// TODO: account for solid velocities
						negativeDivergences(i, j) -= divergenceScale * (uVelocity(i + 1, j) - uVelocity(i, j) + vVelocity(i, j + 1) - vVelocity(i, j));
						UpdateA(Acoefficient, i, j);
						break;
					case GridDataPoint::EMPTY:
						// Assuming empty fluid cell pressure = 0
						break;
				}
			}
		}

		StandardPCG();

		float scale = timeStep / (density * cellWidth);

		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				switch(gridData(i, j).cellState)
				{
					case GridDataPoint::SOLID:
						if(gridData(i - 1, j).cellState == GridDataPoint::FLUID)
						{
							uVelocity(i, j) = 0; // usolid(i, j
						} else
						{
							// Mark uVelocity(i, j) as unknown
						}

						if(gridData(i, j - 1).cellState == GridDataPoint::FLUID)
						{
							vVelocity(i, j) = 0; // vsolid(i, j)
						} else
						{
							// Mark vVelocity(i, j) as unknown
						}
						break;
					case GridDataPoint::FLUID:
						if(gridData(i - 1, j).cellState == GridDataPoint::SOLID)
						{
							uVelocity(i, j) = 0; // usolid(i, j)
						} else
						{
							uVelocity(i, j) -= scale * (pressure(i, j) - pressure(i - 1, j));
						}

						if(gridData(i, j - 1).cellState == GridDataPoint::SOLID)
						{
							vVelocity(i, j) = 0; // vsolid(i, j)
						} else
						{
							vVelocity(i, j) -= scale * (pressure(i, j) - pressure(i, j - 1));
						}
						break;
					case GridDataPoint::EMPTY:
						if(gridData(i - 1, j).cellState == GridDataPoint::FLUID)
						{
							uVelocity(i, j) -= scale * (pressure(i, j) - pressure(i - 1, j));
						} else
						{
							// Mark uVelocity(i, j) as unknown
						}

						if(gridData(i, j - 1).cellState == GridDataPoint::FLUID)
						{
							vVelocity(i, j) -= scale * (pressure(i, j) - pressure(i, j - 1));
						} else
						{
							// Mark vVelocity(i, j) as unknown
						}
				}
			}
		}

	}

	inline unsigned char colourClamp(float max, float val)
	{
		unsigned char t = (val / max) * 255;
		//std::cout << static_cast<unsigned>(t) << std::endl;
		return t;
	}

	void UpdateTexture()
	{
		//float uVelocityMax = *std::max_element(uVelocity.grid.begin(), uVelocity.grid.end());
		//float vVelocityMax = *std::max_element(vVelocity.grid.begin(), vVelocity.grid.end());
		//float pressureMax = *std::max_element(pressure.grid.begin(), pressure.grid.end());
		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				//std::cout << uVelocity(i, j) << std::endl;
				//std::cout << vVelocity(i, j) << std::endl;
				//std::cout << pressure(i, j) << std::endl;
				gridTexture.pixels[i * column + j] = uVelocity(i, j);
				gridTexture.pixels[i * column + j] = vVelocity(i, j);
				gridTexture.pixels[i * column + j] = pressure(i, j);
			}
		}
		fluidRenderComponent->ChangeTextureData(gridTexture.pixels);
	}

	void PrintCell(size_t i, size_t j)
	{
		std::cout << i << ", " << j << std::endl;
		std::cout << "uVelocity " << uVelocity(i, j) << std::endl;
		std::cout << "vVelocity " << vVelocity(i, j) << std::endl;
		std::cout << "Pressure " << pressure(i, j) << std::endl;
		std::cout << std::endl; 
	}
};