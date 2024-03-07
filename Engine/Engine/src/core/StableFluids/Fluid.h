#pragma once
#include <algorithm>

class Fluid
{
protected:
	unsigned int column, row, depth, N;

public:

	float viscosity, diffusionRate;
	int max_iterations;
	float addvel, addsmoke;

	Fluid(const unsigned int _column, const unsigned int _row, const unsigned int _depth) : column(_column), row(_row), depth(_depth), N(std::max(std::max(column, row), depth))
	{
		addvel = 400.f;
		addsmoke = 400.f;
		viscosity = 0.0f;
		diffusionRate = 0.0f;
		max_iterations = 20;
	}

	virtual void Simulate(float timeStep, bool& addForceU, bool& addForceV, bool& addForceW, bool& negAddForceU, bool& negAddForceV, bool& negAddForceW, bool& addSmoke, bool& clear) = 0;

	virtual void Profile(float timeStep, float addForceU, float addForceV, float addForceW, float negAddForceU, float negAddForceV, float negAddForceW, float addSmoke) = 0;

	virtual float* GetSmokeData() = 0;
};