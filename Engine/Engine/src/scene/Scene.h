#pragma once

#include "components/Component.h"
#include <unordered_map>
#include <typeindex>
#include <stdexcept>
#include <memory>
#include "systems/EngineSystem.h"
#include "components/RenderComponent.h"
#include "../types/EngineTypes.h"
#include "systems/RenderSystem.h"

class Scene
{
private:
	/* Number of sparse sets in vector = number of components
	Size of sparse set = number of entities */
	std::unordered_map<std::type_index, std::unique_ptr<BaseEngineSystem>> entities;

	EntityID numberofentities = 0;

	RenderSystem* renderSystem;

public:

	Scene();

	EntityID NewEntity();
	void DeleteEntity(EntityID id);
	void Update(const Matrix4f& cameraMatrix);
	void PrintStatus();

	template<typename ComponentType>
	ComponentType* AddComponent(EntityID id)
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "Type must derive Component");

		if(!entities.contains(typeid(ComponentType)))
		{
			entities[typeid(ComponentType)] = 
				std::make_unique<EngineSystem<ComponentType>>(EngineSystem<ComponentType>());
		}

		return static_cast<ComponentType*>(entities[typeid(ComponentType)].get()->Add(id));
	}

	template<typename ComponentType>
	bool HasComponent(EntityID id)
	{
		// TODO: move these asserts into EngineSystem
		static_assert(std::is_base_of<Component, ComponentType>::value, "Type must derive Component");

		if(entities.contains(typeid(ComponentType)))
		{
			return entities[typeid(ComponentType)].get()->Contains(id);
		}
			
		throw std::logic_error("Component type not found in sparse sets");
		return false;
	}

	template<typename ComponentType>
	ComponentType* GetComponent(EntityID id)
	{
		if(HasComponent<ComponentType>(id))
		{
			return static_cast<ComponentType*>(entities[typeid(ComponentType)].get()->Get(id));
		}
		throw std::logic_error("Entity does not have component");
	} 
};