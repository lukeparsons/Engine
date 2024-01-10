#include "Scene.h"
#include <ranges>
#include "components/TransformComponent.h"
#include "components/RenderComponent.h"
#include "../scene/SystemManager.h"
#include "../renderer/shaders/ShaderStore.h"
#include "../renderer/shaders/LineShader.h"

Scene::Scene()
{
    entities[typeid(TransformComponent)] = std::make_unique<ComponentStore<TransformComponent>>();
    entities[typeid(RenderComponent)] = std::make_unique<ComponentStore<RenderComponent>>();
    //entities[typeid(FluidComponent)] = std::make_unique<ComponentStore<FluidComponent>>();

    systems = std::make_unique<SystemManager>(getStorePointer<TransformComponent>(), getStorePointer<RenderComponent>());
}

Scene::~Scene() = default;

void Scene::Update(const Matrix4f& cameraMatrix)
{
    systems->RunSystems(cameraMatrix);
}

EntityID Scene::NewEntity()
{
    AddTransform(entityIDcount);
    return entityIDcount++;
}

EntityID Scene::CreateModel(std::shared_ptr<Drawable> drawable, const std::shared_ptr<Texture> texture, Vector3f location, Vector3f scale)
{
    EntityID newID = NewEntity();
    TransformComponent* transform = GetComponent<TransformComponent>(newID);
    transform->location = location;
    transform->scale = scale;
    RenderComponent* render = AddComponent<RenderComponent>(newID);
    render->drawable = drawable;
    render->ChangeTexture(texture);
    return newID;
}

EntityID Scene::CreateLine(std::shared_ptr<Drawable> drawable, Vector3f location, Vector3f scale)
{
    EntityID newID = NewEntity();
    TransformComponent* transform = GetComponent<TransformComponent>(newID);
    transform->location = location;
    transform->scale = scale;
    RenderComponent* render = AddComponent<RenderComponent>(newID);
    render->drawable = drawable;
    render->shaderProgram = g_shaderStore.LoadShader<LineShader>("../Engine/src/renderer/shaders/shaderfiles/Line.vertex", "../Engine/src/renderer/shaders/shaderfiles/Line.fragment");
    return newID;
}

void Scene::DeleteEntity(EntityID id)
{
    for(auto& componentSet : entities | std::views::values)
    {
        componentSet->Delete(id);
    }
}

std::unordered_map<std::type_index, Component*> Scene::GetAllEntityComponents(EntityID id) const
{
    std::unordered_map<std::type_index, Component*> entityComponents;
    for(auto const& [type, system] : entities)
    {
        if(system->Contains(id))
        {
            entityComponents[type] = system->Get(id);
        }
    }

    return entityComponents;
}
