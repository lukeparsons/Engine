#pragma once
#include "../Component.h"

class ActiveComponent : public Component
{
public:
	virtual void FrameUpdate() = 0;
};