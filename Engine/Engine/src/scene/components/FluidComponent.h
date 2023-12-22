#pragma once
#include "RenderComponent.h"
#include "../../renderer/shaders/FluidShader.h"
#include "../../core/2DGrid/GridStructures.h"

class FluidComponent : public RenderComponent
{
private:
	float cellWidth;
	GridStructure<float> uVelocity;
public:
	unsigned int column, row;
	std::shared_ptr<FluidShader> fluidShader;

	FluidComponent(EntityID _id);

	void SetShaderValues(const Matrix4f& cameraMatrix, const Matrix4f& modelMatrix);
};