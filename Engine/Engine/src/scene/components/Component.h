#pragma once

#include "../../types/EngineTypes.h"
#include<unordered_map>
#include<typeindex>

class Component
{
public:
	EntityID entity;

	Component(EntityID _entity, std::unordered_map<std::type_index, Component*>& entityComponents) : entity(_entity) {};
};