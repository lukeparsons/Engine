#include "RenderComponent.h"

static const std::shared_ptr<Texture> defaultTexture = std::make_shared<Texture>("../Engine/assets/wall2.png");

RenderComponent::RenderComponent(EntityID _id) : Component(_id), texture(defaultTexture)
{
	// ERROR CHECKING HERE
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); */

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, texture->format, texture->type, texture->pixels.data());

	glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderComponent::ChangeTextureData(const std::vector<float>& pixels)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, texture->format, texture->type, pixels.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderComponent::ChangeTexture(const Texture& _texture)
{
	texture = nullptr;
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, texture->format, _texture.width, _texture.height, 0, texture->format, texture->type, _texture.pixels.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}


void RenderComponent::ChangeTexture(std::shared_ptr<Texture> _texture)
{
	if(texture.get() == _texture.get())
	{
		return;
	}

	texture = _texture;
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, texture->format, _texture->width, _texture->height, 0, texture->format, texture->type, _texture->pixels.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}
