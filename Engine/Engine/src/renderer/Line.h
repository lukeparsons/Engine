#pragma once
#include <array>
#include <glad/glad.h>
#include "../math/Matrix4f.h"
#include "Drawable.h"
#include "shaders/LineShader.h"
#include "shaders/ShaderStore.h"

class Line : public Drawable
{
public:
	GLuint VAO, VBO;
	std::array<float, 6> vertices;

	Line(float startX, float startY, float startZ, float endX, float endY, float endZ)
	{
		
		vertices = { startX, startY, startZ, endX, endY, endZ };
		
		glGenVertexArrays(1, &VAO);

		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

		// Vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	virtual void Draw(const Matrix4f& cameraMatrix, GLuint textureID) const override
	{
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, 2);
		glBindVertexArray(0);
	}

	void ChangeLine(float startX, float startY, float startZ, float endX, float endY, float endZ)
	{
		vertices = { startX, startY, startZ, endX, endY, endZ };
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	~Line()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}
};