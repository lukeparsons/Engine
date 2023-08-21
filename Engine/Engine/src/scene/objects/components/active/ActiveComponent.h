#pragma once
#include "../Component.h"
#include <memory>

class WorldObject;

class ActiveComponent : public Component
{
private:
	virtual ActiveComponent* cloneComponent(WorldObject& newWorldObject) const = 0;
public:
	virtual void FrameUpdate() = 0;

	virtual ~ActiveComponent() {};

	virtual std::unique_ptr<ActiveComponent> Clone(WorldObject& newWorldObject) const
	{
		return std::unique_ptr<ActiveComponent>(cloneComponent(newWorldObject));
	}
};