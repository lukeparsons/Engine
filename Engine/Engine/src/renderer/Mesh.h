#pragma once

#include <vector>
#include "shaders/ShaderProgram.h"
#include <glad/glad.h>
#include "../math/Matrix4f.h"
#include "../math/Vectorf.h"
#include <assimp/scene.h>
#include "materials/Texture.h"

struct Vertex
{
	Vector3f vertex;
	Vector2f tex;
	Vector3f normal;

	Vertex(aiVector3D& aiVertex, aiVector3D& aiTex, aiVector3D& aiNormal) {
		vertex = Vector3f(aiVertex.x, aiVertex.y, aiVertex.z);
		tex = Vector2f(aiTex.x, aiTex.y);
		normal = Vector3f(aiNormal.x, aiNormal.y, aiNormal.z);
	};
};

class Mesh
{
	friend class RenderedObject;
private:
	const ShaderProgram& shaderProgram;
	GLuint textureID;
	GLuint VAO;
	unsigned int transformLoc;
	unsigned int modelLoc;

	void const Draw(const Matrix4f& cameraMatrix, const Vector3f& location, const Vector3f& rotation, const Vector3f& scale);
public:
	Mesh(const char* fileName, const char* textureFileName, const ShaderProgram& shaderProgram);

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

