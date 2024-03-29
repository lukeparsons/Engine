#pragma once

#include "../../renderer/Mesh.h"
#include "TransformComponent.h"

class RenderComponent : public Component {
private:
	std::shared_ptr<Texture> texture;
public:
	std::shared_ptr<Mesh> mesh;
	GLuint textureID;

	RenderComponent(EntityID _id);

	template<typename T>
	inline void ChangeTextureData(const std::vector<T>& pixels)
	{
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, texture->format, texture->type, pixels.data());
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void ChangeTexture(std::shared_ptr<Texture> _texture);
};