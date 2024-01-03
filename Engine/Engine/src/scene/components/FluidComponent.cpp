#include "FluidComponent.h"
#include "../../renderer/shaders/ShaderStore.h"

FluidComponent::FluidComponent(EntityID _id) : RenderComponent(_id), column(100), row(100), cellWidth(0.01f), uVelocity(GridStructure2D<float>(0, 100, 100))
{
	this->texture = std::make_shared<TextureData<unsigned char>>(column, row, GL_RGBA, GL_UNSIGNED_BYTE);
	fluidShader = g_shaderStore.LoadShader<FluidShader>("../Engine/src/renderer/shaders/shaderfiles/fluids/FluidVertex.vertex", "../Engine/src/renderer/shaders/shaderfiles/fluids/FluidFragment.fragment");
}

void FluidComponent::SetShaderValues(const Matrix4f& cameraMatrix, const Matrix4f& modelMatrix)
{
	float timestep = (5 * cellWidth) / uVelocity.max();
	for(unsigned int i = 0; i < column; i++)
	{
		for(unsigned int j = 0; j < row; j++)
		{
			float coords[2] = { i, j };
			fluidShader->Run(cameraMatrix, modelMatrix, timestep, cellWidth, coords, uVelocity(i, j));
		}
	}
}
