#pragma once
#include "GridCell2D.h"
#include "GridStructures.h"
#include "RowVector.h"
#include <map>

static const std::shared_ptr<Texture> fluidTexture = std::make_shared<Texture>("../Engine/assets/smoke.png");
static const std::shared_ptr<Texture> solidTexture = std::make_shared<Texture>("../Engine/assets/block.png");
static const std::shared_ptr<Texture> emptyTexture = std::make_shared<Texture>("../Engine/assets/wall2.png");

template<size_t row, size_t column>
class Grid2D
{
private:
	const float cellWidth;
	EntityID fluidID;
	RenderComponent* fluidRenderComponent;

	Texture texture;

	inline float uvelocity_centre(size_t i, size_t j)
	{
		return (gridData(i - 1, j).uVelocity + gridData(i, j).uVelocity) / 2;
	}

	inline float vvelocity_centre(size_t i, size_t j)
	{
		return (gridData(i, j - 1).vVelocity + gridData(i, j).vVelocity) / 2;
	}

	std::array<float, 4> get_interp_weights(float s) const
	{
		float sSquared = pow(s, 2);
		float sCubed = pow(s, 3);

		float negativeWeight = (-1/3) * s + (1/2) * sSquared - (1/6) * sCubed;
		float weight = 1 - sSquared + (1/2) * (sCubed - s);
		float positiveWeight = s + (1/2) * (sSquared - sCubed);
		float doublePositiveWeight = (1/6) * (sCubed - s);

		return { negativeWeight, weight, positiveWeight, doublePositiveWeight };
	}

	template<typename QuantityType>
	inline float calculate_interp_quantity_i(int i, int j, std::array<float, 4> weights, QuantityType GridDataPoint::* quantity) const
	{
		return weights[0] * gridData(i, j - 1).*quantity + weights[1] * gridData(i, j).*quantity
			+ weights[2] * gridData(i, j + 1).*quantity + weights[3] * gridData(i, j + 2).*quantity;
	}

	template<typename QuantityType>
	inline float calculate_interp_quantity_j(int i, int j, std::array<float, 4> weights, QuantityType GridDataPoint::* quantity)
	{
		return weights[0] * gridData(i - 1, j).*quantity + weights[1] * gridData(i, j).*quantity
			+ weights[2] * gridData(i + 1, j).*quantity + weights[3] * gridData(i + 2, j).*quantity;
	}

public:

	float density;
	float temperature;
	float concentration;

	GridStructure<GridDataPoint, row, column> gridData;
	RowVector<row, column> negativeDivergences;
	RowVector<row, column> precon;

	Grid2D(Scene& scene, std::shared_ptr<Mesh>& gridModel, const Vector2f& location, float _density, float _cellWidth) : density(_density), cellWidth(_cellWidth), texture(Texture(column, row, std::vector<float>(), GL_RGB, GL_FLOAT))
	{
		Vector3f cellScale = Vector3f(cellWidth, cellWidth, cellWidth);

		fluidID = scene.CreateModel(gridModel, std::make_shared<Texture>(texture), Vector3f(location.x, location.y, 0), Vector3f(cellWidth * column * 0.08, cellWidth * row * 0.08, cellWidth));
		fluidRenderComponent = scene.GetComponent<RenderComponent>(fluidID);

		for(unsigned int i = 0; i < column; i++)
		{
			for(unsigned int j = 0; j < row; j++)
			{
				//EntityID cellID = scene.CreateModel(gridModel, fluidTexture,
					//Vector3f(location.x, location.y, 0) + Vector3f(i * cellWidth * 2, j * cellWidth * 2, 0), cellScale);
				//RenderComponent* render = scene.GetComponent<RenderComponent>(cellID);

				gridData.insert(GridDataPoint(Cell2D(), GridDataPoint::FLUID), i, j);
				texture.pixels.push_back(gridData(i, j).uVelocity);
				texture.pixels.push_back(gridData(i, j).vVelocity);
				texture.pixels.push_back(gridData(i, j).pressure);
			}
		}

		// Make the boundary solid
		for(unsigned int i = 0; i < column; i++)
		{
			gridData(i, 0).cellState = GridDataPoint::SOLID;
			//gridData(i, 0).cell.renderComponent->ChangeTexture(solidTexture);
			gridData(i, row - 1).cellState = GridDataPoint::SOLID;
			//gridData(i, row - 1).cell.renderComponent->ChangeTexture(solidTexture);
		}

		for(unsigned int j = 0; j < row; j++)
		{
			gridData(0, j).cellState = GridDataPoint::SOLID;
			//gridData(0, j).cell.renderComponent->ChangeTexture(solidTexture);
			gridData(column - 1, j).cellState = GridDataPoint::SOLID;
			//gridData(column - 1, j).cell.renderComponent->ChangeTexture(solidTexture);
		}
	}

	template<typename QuantityType>
	void advect(float timeStep, QuantityType GridDataPoint::*quantity)
	{
		float scale = 1 / cellWidth;

		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					// Runge-Kutta 2
					int midPointi = static_cast<int>(std::floorf(i - 0.5 * timeStep * gridData(i, j).uVelocity));
					int midPointj = static_cast<int>(std::floorf(j - 0.5 * timeStep * gridData(i, j).vVelocity));
					//std::cout << "midpoint (" << midPointi << ", " << midPointj << ")" << std::endl;
					if(gridData.snap_to_grid(midPointi, midPointj))
					{
						gridData(i, j).*quantity = gridData(midPointi, midPointj).*quantity;
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

					if(gridData.snap_to_grid(originali, originalj))
					{
						gridData(i, j).*quantity = gridData(originali, originalj).*quantity;
						continue;
					}

					//Cubic interpolation

					// x-axis
					float negativeQi = calculate_interp_quantity_i(originali - 1, originalj, weightsi, quantity);
					float Qi = calculate_interp_quantity_i(originali, originalj, weightsi, quantity);
					float positiveQi = calculate_interp_quantity_i(originali + 1, originalj, weightsi, quantity);
					float doublePositiveQi = calculate_interp_quantity_i(originali + 2, originalj, weightsi, quantity);

					float finalQi = weightsi[0] * negativeQi + weightsi[1] * Qi + weightsi[2] * positiveQi + weightsi[3] * doublePositiveQi;

					// y-axis
					float negativeQj = calculate_interp_quantity_j(originali, originalj - 1, weightsj, quantity);
					float Qj = calculate_interp_quantity_j(originali, originalj, weightsj, quantity);
					float positiveQj = calculate_interp_quantity_j(originali, originalj + 1, weightsj, quantity);
					float doublePositiveQj = calculate_interp_quantity_j(originali, originalj + 2, weightsj, quantity);

					float finalQj = weightsj[0] * negativeQj + weightsj[1] * Qj + weightsj[2] * positiveQj + weightsj[3] * doublePositiveQj;

					float averageQ = (finalQi + finalQj) / 2;
					gridData(i, j).*quantity = averageQ;
					//gridData(i, j).*quantity = averageQ >= 0 ? averageQ : 0;

				}

			}
		}
	}

	template<typename QuantityType>
	void addforce(float timeStep, float force, size_t i, size_t j, QuantityType GridDataPoint::*quantity)
	{
		if(gridData(i, j).cellState == GridDataPoint::FLUID)
		{
			gridData(i, j).*quantity += timeStep * force;
		}
	}

	double StandardPCG()
	{
		// PCG algorithm for solving Ap = b
		double pressureGuess = 0;

		RowVector<row, column> residualVector = negativeDivergences;
		RowVector<row, column> auxiliaryVector;

		applyPreconditioner(residualVector, auxiliaryVector);
		RowVector<row, column> searchVector = auxiliaryVector;

		double sigma = DotProduct(auxiliaryVector, residualVector);

		// TODO: Implement proper scaling
		double tolerance = 0.00001;

		for(unsigned int i = 0; i < 200; i++) // 200 here is max iterations
		{
			applyA(searchVector, auxiliaryVector);
			double alpha = sigma / DotProduct(auxiliaryVector, searchVector);
			pressureGuess = pressureGuess + alpha * sigma;
			residualVector = residualVector - (alpha * auxiliaryVector);

			if(residualVector.max() <= tolerance)
			{
				return pressureGuess;
			}

			applyPreconditioner(residualVector, auxiliaryVector);
			double sigmaNew = DotProduct(auxiliaryVector, residualVector);
			double beta = sigmaNew / sigma;

			searchVector = auxiliaryVector + (beta * searchVector);

			sigma = sigmaNew;
		}

		// Iteration limit exceeded
		return pressureGuess;
	}

	void applyA(const RowVector<row, column>& vector, RowVector<row, column>& result)
	{
		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				result(i, j) = gridData(i, j).Adiag * vector(i, j)
					+ gridData(i - 1, j).Ax * vector(i - 1, j)
					+ gridData(i, j).Ax * vector(i + 1, j)
					+ gridData(i, j).Ay * vector(i, j + 1)
					+ gridData(i, j - 1).Ay * vector(i, j - 1);
			}
		}
	}

	/* Modified Incomplete Choleksy preconditoner to find a matrix M where M is approx A^ {-1}
	 Simple to implement, fairly efficient and robust in handling irregular domains (e.g liquid splash)
	 However hard to efficiently parallelize and not optimally scalable
	*/
	void applyPreconditioner(RowVector<row, column>& residualVector, RowVector<row, column>& result)
	{
		double tuningConstant = 0.97;
		double safetyConstant = 0.25;

		double e = 0;
		double cellPressureCoefficient = 0;
		double t = 0;

		double previousXPrecon = 0;
		double previousYPrecon = 0;
		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					double Aplusi = gridData(i - 1, j).Ax;
					double Aplusj = gridData(i, j - 1).Ay;

					double cellPressureCoefficient = gridData(i, j).Adiag;

					e = cellPressureCoefficient - pow((Aplusi * previousXPrecon), 2)
						- pow((Aplusj - 1) * previousYPrecon, 2);

					e -= tuningConstant * (Aplusi * (Aplusj * pow(previousXPrecon, 2))
						+ Aplusj * (Aplusi * previousYPrecon, 2));

					if(e < safetyConstant * cellPressureCoefficient)
					{
						e = cellPressureCoefficient;
					}

					precon(i, j) = 1 / sqrt(e);

					// Solve Lq = r
					t = residualVector(i, j)
						- Aplusi * previousXPrecon * gridData(i - 1, j).q
						- Aplusj * previousYPrecon * gridData(i - 1, j - 1).q;

					gridData(i, j).q = t * precon(i, j);

					// NOTE: These will be the same as the precon from the last iteration of applyPreconditoner
					double previousXPrecon = precon(i, j);
					double previousYPrecon = precon(i, j);
				}
			}
		}

		for(size_t i = column - 1; i-- > 0;)
		{
			for(size_t j = row - 1; j-- > 0;)
			{
				if(gridData(i, j).cellState == GridDataPoint::FLUID)
				{
					t = gridData(i, j).q
						- gridData(i, j).Ax * precon(i, j) * gridData(i + 1, j).z
						- gridData(i, j).Ay * precon(i, j) * gridData(i, j + 1).z;
				}

				gridData(i, j).z = t * precon(i, j);
			}
		}
		result = precon;
	}

	void UpdatePressure(float Acoefficient, GridDataPoint& cellData, size_t i, size_t j)
	{

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
	void Update(float timeStep)
	{
		float scale = timeStep / (density * cellWidth);
		float divergenceScale = 1 / cellWidth;
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

						if(gridData(i - 1, j).cellState == GridDataPoint::SOLID)
						{
							// gridData(i, j) = usolid(i, j)
						} else
						{
							gridData(i, j).uVelocity -= scale * (gridData(i, j).pressure - gridData(i - 1, j).pressure);
						}

						if(gridData(i, j - 1).cellState == GridDataPoint::SOLID)
						{
							// gridData(i, j) = vsolid(i, j)
						} else
						{
							gridData(i, j).vVelocity -= scale * (gridData(i, j).pressure - gridData(i, j - 1).pressure);
						}

						// TODO: for solid velocities
						negativeDivergences(i, j) = -divergenceScale * (gridData(i + 1, j).uVelocity - gridData(i, j).uVelocity +
							gridData(i, j + 1).vVelocity - gridData(i, j).vVelocity);

						UpdatePressure(Acoefficient, gridData(i, j), i, j);

						break;
					case GridDataPoint::EMPTY:
						// Assuming empty fluid cell pressure = 0
						if(gridData(i - 1, j).cellState == GridDataPoint::FLUID)
						{
							gridData(i, j).uVelocity -= scale * -gridData(i - 1, j).pressure;
						} else
						{
							gridData(i, j).vVelocity -= scale * -gridData(i, j - 1).pressure;
						}
						break;
				}
			}
		}

		StandardPCG();

		/*for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				GLuint shaderID = gridData(i, j).cell.GetShaderID();
				glUseProgram(shaderID);
				GLfloat colour[4] = { 0.5f, 0.5f, 0.5f, 1.0f};
				glUniform4fv(glGetUniformLocation(shaderID, "FluidColour"), 1, colour);
			}
		} */

		//std::cout << gridData(5, 5).uVelocity << std::endl;
	}

	void UpdateTexture()
	{
		for(size_t i = 0; i < column; i++)
		{
			for(size_t j = 0; j < row; j++)
			{
				//std::cout << gridData(i, j).uVelocity << std::endl;
				//std::cout << gridData(i, j).vVelocity << std::endl;
				//std::cout << gridData(i, j).pressure << std::endl;
				texture.pixels[i * column + j] = 10;
				texture.pixels[i * column + j] = 10;
				texture.pixels[i * column + j] = 10;
			}
		}
		fluidRenderComponent->ChangeTextureData(texture.pixels);
	}

	void PrintCell(size_t i, size_t j)
	{
		std::cout << i << ", " << j << std::endl;
		std::cout << "uVelocity " << gridData(i, j).uVelocity << std::endl;
		std::cout << "vVelocity " << gridData(i, j).vVelocity << std::endl;
		std::cout << "Pressure " << gridData(i, j).pressure << std::endl;
		std::cout << std::endl;
	}
};