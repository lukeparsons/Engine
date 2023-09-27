#pragma once

#include "components/Component.h"
#include "components/TransformComponent.h"
#include <unordered_map>
#include <typeindex>
#include <stdexcept>
#include <memory>
#include "../types/EngineTypes.h"
#include "ComponentStore.h"
#include "../renderer/Mesh.h"

class SystemManager;

class Scene
{
private:
	/* Number of sparse sets in vector = number of components
	Size of sparse set = number of entities */
	std::unordered_map<std::type_index, std::unique_ptr<BaseComponentStore>> entities;

	EntityID entityIDcount = 0;

	std::unique_ptr<SystemManager> systems;

	template<typename T>
	ComponentStore<T>* getStorePointer()
	{
		return static_cast<ComponentStore<T>*>(entities[typeid(T)].get());
	}

	bool entityexists(EntityID id)
	{
		return entities[typeid(TransformComponent)]->Contains(id);
	}

	void AddTransform(EntityID id)
	{
		entities[typeid(TransformComponent)]->Add(id);
	}

public:

	Scene();
	~Scene();

	EntityID NewEntity();
	EntityID CreateModel(std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Texture>& texture, Vector3f location = Vector3f(0, 0, 0), Vector3f scale = Vector3f(1, 1, 1));
	void DeleteEntity(EntityID id);
	std::unordered_map<std::type_index, Component*> GetAllEntityComponents(EntityID id) const;

	void Update(const Matrix4f& cameraMatrix);

	template<typename ComponentType>
	ComponentType* AddComponent(EntityID id)
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "Type must derive Component");

		if(entityexists(id))
		{
			if(!entities.contains(typeid(ComponentType)))
			{
				entities[typeid(ComponentType)] =
					std::make_unique<ComponentStore<ComponentType>>(ComponentStore<ComponentType>());
			}

			if(entities[typeid(ComponentType)]->Contains(id))
			{
				throw std::logic_error("Entity already has component");
			}
			return static_cast<ComponentType*>(entities[typeid(ComponentType)]->Add(id));
		}

		throw std::logic_error("Entity does not exist");
	}
	
	template<typename ComponentType>
	bool HasComponent(EntityID id)
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "Type must derive from Component");

		if(entityexists(id))
		{
			if(entities.contains(typeid(ComponentType)))
			{
				return entities[typeid(ComponentType)]->Contains(id);
			}

			throw std::logic_error("Component type not found in sparse sets");
		}
		throw std::logic_error("Entity does not exist");
	}

	template<typename ComponentType>
	ComponentType* GetComponent(EntityID id)
	{
		if(HasComponent<ComponentType>(id))
		{
			return static_cast<ComponentType*>(entities[typeid(ComponentType)]->Get(id));
		}
		throw std::logic_error("Entity does not have component");
	} 
};