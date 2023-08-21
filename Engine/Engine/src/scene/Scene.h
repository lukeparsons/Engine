#pragma once
#include <set>
#include "objects/WorldObject.h"
#include <memory>

struct ComponentCompare
{
	bool operator()(const std::shared_ptr<WorldObject>& lhs, const std::shared_ptr<WorldObject>& rhs) const
	{
		return lhs.get()->activeComponents.size() > rhs.get()->activeComponents.size();
	}
};

class Scene
{
private:
	size_t numberActiveObjects = 0;
	std::multiset<std::shared_ptr<WorldObject>, ComponentCompare> worldObjects;
public:
	inline std::multiset<std::shared_ptr<WorldObject>, ComponentCompare> GetWorldObjects() const
	{
		return worldObjects;
	}

	void AddWorldObject(const WorldObject& worldObject);
	//void AddWorldObject(WorldObject&& worldObject);
	void DeleteWorldObject(std::shared_ptr<WorldObject>& worldObject);

	void FrameUpdateActiveObjects() const;
};

