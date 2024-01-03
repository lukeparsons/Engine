#pragma once
#include "../../types/EngineTypes.h"
#include<unordered_map>
#include<typeindex>

class Component
{
public:
	EntityID entity;

	Component(EntityID _entity) : entity(_entity) {};
};