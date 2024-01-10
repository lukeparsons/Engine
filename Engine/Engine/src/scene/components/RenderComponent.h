#pragma once
#include "../../renderer/Mesh.h"
#include "TransformComponent.h"
#include "../../renderer/shaders/BasicShader.h"

class RenderComponent : public Component {
protected:
	std::shared_ptr<Texture> texture;
public:
	std::shared_ptr<Drawable> drawable;
	GLuint textureID;
	std::shared_ptr<ShaderProgram> shaderProgram;

	bool isActive;

	RenderComponent(EntityID _id);

	template<typename T>
	inline void ChangeTextureData(const std::vector<T>& pixels)
	{
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, texture->format, texture->type, pixels.data());
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void ChangeTexture(std::shared_ptr<Texture> _texture);

	void SetColour(std::array<float, 3> colour);
};