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
    // TODO: Add rvalue AddComponent!
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

void Scene::PrintStatus()
{
    size_t i = 0;
    for(auto it = entities.begin(); it != entities.end(); it++)
    {
        std::cout << "System " << it->first.name() << " initialized" << std::endl;
    }
}
