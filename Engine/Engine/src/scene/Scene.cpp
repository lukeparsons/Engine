#include "Scene.h"
#include <ranges>
#include "components/TransformComponent.h"

Scene::Scene()
{
    entities[typeid(TransformComponent)] = std::make_unique<ComponentStore<TransformComponent>>();
    entities[typeid(RenderComponent)] = std::make_unique<ComponentStore<RenderComponent>>();
    renderSystem = std::make_unique<RenderSystem>(GetStorePointer<TransformComponent>(), GetStorePointer<RenderComponent>());
}

EntityID Scene::NewEntity()
{
    AddComponent<TransformComponent>(numberofentities);
    return numberofentities++;
}

EntityID Scene::CreateModel(const Mesh& mesh, Vector3f location, Vector3f scale)
{
    EntityID newID = NewEntity();
    TransformComponent* transform = GetComponent<TransformComponent>(newID);
    transform->location = location;
    transform->scale = scale;
    RenderComponent* render = AddComponent<RenderComponent>(newID);
    render->mesh = &mesh;
    return newID;
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
