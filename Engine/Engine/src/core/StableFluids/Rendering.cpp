#include "StableFluids.h"

void StableFluids::InitModelRender()
{
	meshShader = g_shaderStore.LoadShader<BasicShader>("../Engine/src/renderer/shaders/shaderfiles/BasicVertex.vertex", "../Engine/src/renderer/shaders/shaderfiles/BasicFragment.fragment");
	glUseProgram(meshShader->GetID());

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, solidTexture->width, solidTexture->height, 0, solidTexture->format, solidTexture->type, solidTexture->GetPixelData());
}

void StableFluids::ModelRender(Matrix4f& cameraMatrix)
{
	Matrix4f modelMatrix;
	Vector3f scale = Vector3f(0.01, 0.01, 0.01);
	float x, y, z;
	float h = 1.0f / N;
	for(int i = 1; i <= column; i++)
	{
		x = (i - 0.5f) * h;
		for(int j = 1; j <= row; j++)
		{
			y = (j - 0.5f) * h;
			for(int k = 1; k <= depth; k++)
			{
				z = (k - 0.5f) * h;
				if(smoke(i, j, k) >= 0.1f)
				{
					modelMatrix = GetTranslationMatrix(Vector3f(x, y, z)) * GetScaleMatrix(scale);
					meshShader->Configure(cameraMatrix, modelMatrix);

					cellMesh->QuickDraw();
				}
			}
		}
	}
}

void StableFluids::InitVelocityRender()
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

void StableFluids::VelocityRender(Matrix4f& cameraMatrix)
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

				if(uVelocity(i, j, k) >= tol || uVelocity(i, j, k) <= -tol)
				{
					endX = x;
				} else
				{
					endX = x + uVelocity(i, j, k);
				}

				if(vVelocity(i, j, k) >= tol || vVelocity(i, j, k) <= -tol)
				{
					endY = y;
				} else
				{
					endY = y + vVelocity(i, j, k);
				}

				if(wVelocity(i, j, k) >= tol || wVelocity(i, j, k) <= -tol)
				{
					endZ = z;
				} else
				{
					endZ = z + wVelocity(i, j, k);
				}

				vertices = { x, y, z, endX, endY, endZ };
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

				glDrawArrays(GL_LINES, 0, 2);


			}
		}
	}
}