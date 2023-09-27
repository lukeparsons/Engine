#include "RenderComponent.h"

static const std::shared_ptr<Texture> defaultTexture = std::make_shared<Texture>("../Engine/assets/wall2.png");

RenderComponent::RenderComponent(EntityID _id) : Component(_id), texture(defaultTexture)
{
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texture->data[0]);

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
	glTexImage2D(GL_TEXTURE_2D, 0, 4, _texture->width, _texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &_texture->data[0]);
	glBindTexture(GL_TEXTURE_2D, 0);
}
