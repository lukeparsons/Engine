#include "OpenCLFluids.h"
#include "../../../renderer/shaders/ShaderStore.h"
#include "../../../renderer/shaders/LineShader.h"

#define IX(i,j,k) ((i)+(column+2)*(j) + (column+2)*(row+2)*(k))

void OpenCLFluids::InitVelocityRender()
{
	lineShader = g_shaderStore.LoadShader<LineShader>("../Engine/src/renderer/shaders/shaderfiles/Line.vertex", "../Engine/src/renderer/shaders/shaderfiles/Line.fragment");
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glUseProgram(lineShader->GetID());
	float colour[3] = { 0.0f, 0.9f, 0.1f };
	glUniform3fv(lineShader->colourLoc, 1, colour);
}

void OpenCLFluids::VelocityRender(Matrix4f& cameraMatrix)
{
	float h = 1.0f / N;
	float x, y, z;
	float tol = 0.2f;
	float endX, endY, endZ;

	glUniformMatrix4fv(lineShader->transformLoc, 1, GL_TRUE, cameraMatrix.matrix[0]);
	for(int i = 1; i <= column; i++)
	{
		x = (i - 0.5f) * h;
		for(int j = 1; j <= row; j++)
		{
			y = (j - 0.5f) * h;
			for(int k = 1; k <= depth; k++)
			{
				z = (k - 0.5f) * h;

				if(uVelocity[IX(i, j, k)] >= tol || uVelocity[IX(i, j, k)] <= -tol)
				{
					endX = x;
				} else
				{
					endX = x + uVelocity[IX(i, j, k)];
				}

				if(vVelocity[IX(i, j, k)] >= tol || vVelocity[IX(i, j, k)] <= -tol)
				{
					endY = y;
				} else
				{
					endY = y + vVelocity[IX(i, j, k)];
				}

				if(wVelocity[IX(i, j, k)] >= tol || wVelocity[IX(i, j, k)] <= -tol)
				{
					endZ = z;
				} else
				{
					endZ = z + wVelocity[IX(i, j, k)];
				}

				vertices = { x, y, z, endX, endY, endZ };
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

				glDrawArrays(GL_LINES, 0, 2);


			}
		}
	}
}