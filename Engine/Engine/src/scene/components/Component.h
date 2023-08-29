#pragma once

#include "../../types/EngineTypes.h"

class Component
{
public:
	EntityID entity;

	Component(EntityID _entity) : entity(_entity) {};
};