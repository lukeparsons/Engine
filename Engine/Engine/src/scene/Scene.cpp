#include "Scene.h"
#include <ranges>
#include "components/TransformComponent.h"

Scene::Scene()
{
    entities[typeid(RenderComponent)] = std::make_unique<RenderSystem>();
    renderSystem = static_cast<RenderSystem*>(entities[typeid(RenderComponent)].get());
}

EntityID Scene::NewEntity()
{
    AddComponent<TransformComponent>(numberofentities);
    return numberofentities++;
}

void Scene::DeleteEntity(EntityID id)
{
    for(auto& componentSet : entities | std::views::values)
    {
        componentSet.get()->Delete(id);
    }
}

void Scene::Update(const Matrix4f& cameraMatrix)
{
    renderSystem->Render(cameraMatrix);
}

std::unordered_map<std::type_index, Component*> Scene::GetAllEntityComponents(EntityID id) const
{
    std::unordered_map<std::type_index, Component*> entityComponents;
    for(auto const& [type, system] : entities)
    {
        if(system.get()->Contains(id))
        {
            entityComponents[type] = system.get()->Get(id);
        }
    }

    return entityComponents;
}


void Scene::PrintStatus()
{
    size_t i = 0;
    for(auto it = entities.begin(); it != entities.end(); it++)
    {
        std::cout << "System " << it->first.name() << " initialized" << std::endl;
    }
}
