#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>
#include <iostream>
#include "shaders/Shader.h"
#include <filesystem>
#include <vector>
#include "Texture.h"
#include <cassert>

static Assimp::Importer importer;

void Mesh::readmesh(const char* fileName)
{
	const aiScene* scene = importer.ReadFile(fileName,
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

	if(scene == nullptr)
	{
		std::cout << importer.GetErrorString() << std::endl;
	}

	for(unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];
		for(unsigned int j = 0; j < mesh->mNumVertices; j++)
		{
			aiVector3D texVector;
			if(mesh->HasTextureCoords(0))
			{
				texVector = mesh->mTextureCoords[0][j];
			} else
			{
				texVector = aiVector3D(0.0f, 0.0f, 0.0f);
			}
			Vertex vertex(mesh->mVertices[j], texVector, mesh->mNormals[j]);
			vertices.push_back(vertex);
		}

		for(unsigned int k = 0; k < mesh->mNumFaces; k++)
		{
			aiFace& face = mesh->mFaces[k];
			if(face.mNumIndices == 3)
			{
				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			} else
			{
				std::cerr << "Error loading model " << fileName << ", index count on face not equal to 3" << std::endl;
			}
		}
	}
}

void Mesh::constructmesh(ShaderProgram* const shaderProgram)
{
	GLuint VBO, EBO;
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

	// Vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

	// Tex coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));

	// Normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 5));

	glBindVertexArray(0);

	transformLoc = glGetUniformLocation(shaderProgram->GetID(), "transform");
	modelLoc = glGetUniformLocation(shaderProgram->GetID(), "mesh");
	textureLoc = glGetUniformLocation(shaderProgram->GetID(), "texture");
}

Mesh::Mesh(const char* fileName, ShaderProgram *const shaderProgram) : shaderProgram(shaderProgram)
{
	readmesh(fileName);
	constructmesh(shaderProgram);
}

Mesh::Mesh(const Mesh& other)
{
	vertices = other.vertices;
	indices = other.indices;
	shaderProgram = other.shaderProgram;
	constructmesh(shaderProgram);
}

void Mesh::Draw(const Matrix4f& cameraMatrix, GLuint textureID, const Vector3f& location, const Vector3f& rotation, const Vector3f& scale) const
{
	glUseProgram(shaderProgram->GetID());
	glUniformMatrix4fv(transformLoc, 1, GL_TRUE, cameraMatrix.matrix[0]);


	// TODO: Include rotation matrix
	Matrix4f modelMatrix = GetTranslationMatrix(location) * GetScaleMatrix(scale);
	glUniformMatrix4fv(modelLoc, 1, GL_TRUE, modelMatrix.matrix[0]);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glBindVertexArray(VAO);

	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}