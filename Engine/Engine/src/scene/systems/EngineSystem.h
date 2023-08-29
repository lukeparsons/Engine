#pragma once

#include <stdexcept>
#include <iostream>
#include <vector>
#include <assert.h>
#include "../../types/EngineTypes.h"
#include "../components/Component.h"

class BaseEngineSystem
{
public:
	virtual Component* Add(EntityID id) = 0;

	virtual void Delete(EntityID id) = 0;

	virtual Component* Get(EntityID id) = 0;

	virtual bool Contains(EntityID id) = 0;
};

template<typename ComponentType>
class EngineSystem : public BaseEngineSystem
{
protected:
	std::vector<EntityID> sparse;
	std::vector<ComponentType> dense;
public:

	EngineSystem()
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "EngineSystem ComponentType must inherit from Component");
		sparse = std::vector<EntityID>();
		dense = std::vector<ComponentType>();
	}

	// TODO: Check dupes and possible accidential sparse to dense mapping
	// Also deleting then creating new entity in same place can currently cause bugs
	Component* Add(EntityID id)
	{
		if(id < 0)
		{
			throw std::out_of_range("Trying to insert value out of sparse set range");
		}

		dense.push_back(ComponentType(id));

		if(id + 1 < std::numeric_limits<EntityID>::max())
		{
			if(sparse.size() < static_cast<size_t>(id + 1))
			{
				sparse.resize(static_cast<size_t>(id + 1));
			}
		} else
		{
			throw std::overflow_error("Size of sparse set exceeds maximum value supported by set type");
		}

		sparse[id] = static_cast<EntityID>(dense.size() - 1);
		return &dense[dense.size() - 1];
	}

	bool Contains(EntityID id)
	{
		if(id < sparse.size() && id >= 0)
		{
			EntityID denseIndex = sparse[id];
			return denseIndex >= 0 && denseIndex < dense.size() && dense[denseIndex].entity == id;
		}
		return false;
	}

	virtual Component* Get(EntityID id)
	{
		return &dense[id];
	}

	void Delete(EntityID id)
	{
		if(Contains(id))
		{
			EntityID denseIndex = sparse[id]; // Get index in dense array of id

			EntityID lastAddedSparseIndex = dense.back().entity; // Get the index in the sparse array of the last id added to dense array

			dense[denseIndex].entity = lastAddedSparseIndex; // Replace value in dense array with last id added to dense array

			sparse[lastAddedSparseIndex] = denseIndex; // Update value in sparse index with new position in dense index

			dense.pop_back();

			dense.shrink_to_fit();

			EntityID maxDenseVal = std::max_element(dense.begin(), dense.end(),
				[](const ComponentType& c1, const ComponentType& c2) { return c1.entity < c2.entity; })->entity;

			if(sparse[maxDenseVal] == denseIndex)
			{
				sparse.resize(static_cast<size_t>(maxDenseVal + 1));
				sparse.shrink_to_fit();
			}
		}
	}

	void Clear()
	{
		dense.clear();
		sparse.clear();
	}

	void Print()
	{
		std::cout << "Dense array " << std::endl;
		for(EntityID i = 0; i < dense.size(); i++)
		{
			std::cout << dense[i] << " ";
		}
		std::cout << std::endl;

		std::cout << "Sparse array" << std::endl;
		for(EntityID i = 0; i < sparse.size(); i++)
		{
			if(sparse[i] >= 0 && sparse[i] < dense.size() && i == dense[sparse[i]])
			{
				std::cout << sparse[i] << " ";
			} else
			{
				std::cout << "X ";
			}
		}
		std::cout << std::endl;
	}
};