#include "../../../include/ECS/Systems/ECSRenderSystem.h"
#include "../../../include/Renderer/Mesh.h"
#include "../../../include/Renderer/Material.h"
#include "../../../include/Renderer/Graphics.h"
#include "../../../include/Events/ECSEvents.h"
#include "../../../include/Events/EventBus.h"
#include "../../../include/ECS/Components.h"
#include "../../../include/Utils/Logger.h"
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>

using namespace DirectX;

namespace ECS {

void RenderSystem::Init() {
    if (!m_eventBus) {
        LOG_ERROR("RenderSystem: EventBus is null in Init!");
        return;
    }
    LOG_INFO("RenderSystem: Initialized and subscribing to events.");

    // Subscribe to ComponentAdded events
    m_eventSubscriptions.push_back(
        m_eventBus->Subscribe(EventType::ComponentAdded, [this](Event& e) {
            auto& event = static_cast<ComponentAddedEvent&>(e);
            // Check if relevant component
            if (event.componentType == typeid(RenderComponent) || 
                event.componentType == typeid(TransformComponent)) {
                OnComponentAdded(event.entity);
            }
        })
    );

    // Subscribe to ComponentRemoved events
    m_eventSubscriptions.push_back(
        m_eventBus->Subscribe(EventType::ComponentRemoved, [this](Event& e) {
            auto& event = static_cast<ComponentRemovedEvent&>(e);
            if (event.componentType == typeid(RenderComponent) ||
                event.componentType == typeid(TransformComponent)) {
                OnComponentRemoved(event.entity);
            }
        })
    );
}

void RenderSystem::Shutdown() {
    if (!m_eventBus) return;
    
    for (auto id : m_eventSubscriptions) {
        m_eventBus->Unsubscribe(EventType::ComponentAdded, id);
        m_eventBus->Unsubscribe(EventType::ComponentRemoved, id);
    }
    m_eventSubscriptions.clear();
}

void RenderSystem::Update(float deltaTime) {
    UpdateRenderCache();
}

void RenderSystem::Render(Renderer* renderer, Camera& camera, const DirectionalLight& dirLight) {
    if (!renderer) return;
    
    // Gather lights
    std::vector<PointLight> lights;
    auto lightEntities = m_componentManager.QueryEntities<LightComponent, TransformComponent>();
    
    for (Entity entity : lightEntities) {
        auto& light = m_componentManager.GetComponent<LightComponent>(entity);
        auto& transform = m_componentManager.GetComponent<TransformComponent>(entity);
        
        if (!light.enabled) continue;
        
        PointLight pl;
        pl.position = DirectX::XMFLOAT4(transform.position.x, transform.position.y, transform.position.z, light.range);
        pl.color = light.color;
        pl.attenuation = DirectX::XMFLOAT4(1.0f, 0.09f, 0.032f, 0.0f); 
        lights.push_back(pl);
    }

    // Build instances from cache
    // Build instances from cache
    std::vector<const Renderer::RenderInstance*> instances;
    instances.reserve(m_renderCache.size());
    
    for (const auto& entry : m_renderCache) {
        instances.push_back(&entry.instance);
    }

    // Render scene
    renderer->RenderFrame(camera, instances, dirLight, lights);
}

void RenderSystem::RenderDebug(Renderer* renderer, Camera& camera) {
    if (!renderer) return;
    
    // Get all entities with Collider and Transform
    std::vector<AABB> aabbs;
    
    auto entities = m_componentManager.QueryEntities<ColliderComponent, TransformComponent>();
    
    for (Entity entity : entities) {
        auto& collider = m_componentManager.GetComponent<ColliderComponent>(entity);
        
        if (!collider.enabled) continue;
        
        auto& transform = m_componentManager.GetComponent<TransformComponent>(entity);
        
        // Calculate world-space AABB
        AABB worldAABB;
        worldAABB.extents.x = transform.scale.x * collider.localAABB.extents.x;
        worldAABB.extents.y = transform.scale.y * collider.localAABB.extents.y;
        worldAABB.extents.z = transform.scale.z * collider.localAABB.extents.z;
        
        worldAABB.center.x = transform.position.x + (transform.scale.x * collider.localAABB.center.x);
        worldAABB.center.y = transform.position.y + (transform.scale.y * collider.localAABB.center.y);
        worldAABB.center.z = transform.position.z + (transform.scale.z * collider.localAABB.center.z);
        
        aabbs.push_back(worldAABB);
    }
    
    renderer->RenderDebugAABBs(camera, aabbs);
}

// ==================================================================================
// Cache Management
// ==================================================================================

void RenderSystem::RebuildRenderCache()
{
    m_renderCache.clear();
    m_entityToRenderCacheIndex.clear();

    auto renderArray = m_componentManager.GetComponentArray<RenderComponent>();
    auto& renderVec = renderArray->GetComponentArray();
    
    m_renderCache.reserve(renderVec.size());

    for (size_t i = 0; i < renderVec.size(); ++i) {
        Entity entity = renderArray->GetEntityAtIndex(i);
        RenderComponent& render = renderVec[i];
        
        if (!render.mesh || !render.material) continue;
        
        if (!m_componentManager.HasComponent<TransformComponent>(entity)) continue;
        TransformComponent& transform = m_componentManager.GetComponent<TransformComponent>(entity);
        
        CreateRenderCacheEntry(entity, &transform, &render);
    }
}

void RenderSystem::UpdateRenderCache()
{
    for (size_t i = 0; i < m_renderCache.size();)
    {
        Entity entity = m_renderCache[i].entity;
        
        // Check if components still exist
        if (!m_componentManager.HasComponent<TransformComponent>(entity) ||
            !m_componentManager.HasComponent<RenderComponent>(entity))
        {
            RemoveRenderCacheEntry(i);
            continue;
        }
        
        auto& transform = m_componentManager.GetComponent<TransformComponent>(entity);
        auto& render = m_componentManager.GetComponent<RenderComponent>(entity);

        if (!render.mesh || !render.material)
        {
            RemoveRenderCacheEntry(i);
            continue;
        }

        bool transformChanged =
            transform.position.x != m_renderCache[i].lastPosition.x ||
            transform.position.y != m_renderCache[i].lastPosition.y ||
            transform.position.z != m_renderCache[i].lastPosition.z ||
            transform.rotation.x != m_renderCache[i].lastRotation.x ||
            transform.rotation.y != m_renderCache[i].lastRotation.y ||
            transform.rotation.z != m_renderCache[i].lastRotation.z ||
            transform.scale.x != m_renderCache[i].lastScale.x ||
            transform.scale.y != m_renderCache[i].lastScale.y ||
            transform.scale.z != m_renderCache[i].lastScale.z;

        bool renderChanged =
            render.mesh != m_renderCache[i].instance.mesh ||
            render.material.get() != m_renderCache[i].instance.material;

        if (transformChanged || renderChanged)
        {
            RefreshRenderCacheEntry(i, &transform, &render);
        }

        ++i;
    }
}

void RenderSystem::RemoveRenderCacheEntry(size_t index)
{
    if (index >= m_renderCache.size()) return;

    Entity entity = m_renderCache[index].entity;
    m_entityToRenderCacheIndex.erase(entity);

    size_t lastIndex = m_renderCache.size() - 1;
    if (index != lastIndex)
    {
        std::swap(m_renderCache[index], m_renderCache[lastIndex]);
        m_entityToRenderCacheIndex[m_renderCache[index].entity] = index;
    }

    m_renderCache.pop_back();
}

void RenderSystem::RefreshRenderCacheEntry(size_t index, const TransformComponent* transform, const RenderComponent* render)
{
    auto& entry = m_renderCache[index];
    entry.instance.mesh = render->mesh;
    entry.instance.material = render->material.get();
    entry.instance.position = transform->position;
    entry.instance.rotation = transform->rotation;
    entry.instance.scale = transform->scale;
    entry.instance.hasBounds = TryComputeWorldBounds(entry.entity, transform, entry.instance);

    entry.lastPosition = transform->position;
    entry.lastRotation = transform->rotation;
    entry.lastScale = transform->scale;
}

void RenderSystem::CreateRenderCacheEntry(Entity entity, const TransformComponent* transform, const RenderComponent* render)
{
    RenderCacheEntry entry;
    entry.entity = entity;
    entry.instance.mesh = render->mesh;
    entry.instance.material = render->material.get();
    entry.instance.position = transform->position;
    entry.instance.rotation = transform->rotation;
    entry.instance.scale = transform->scale;
    entry.instance.hasBounds = TryComputeWorldBounds(entity, transform, entry.instance);
    entry.lastPosition = transform->position;
    entry.lastRotation = transform->rotation;
    entry.lastScale = transform->scale;

    m_entityToRenderCacheIndex[entity] = m_renderCache.size();
    m_renderCache.push_back(entry);
}

bool RenderSystem::TryComputeWorldBounds(Entity entity, const TransformComponent* transform, Renderer::RenderInstance& instance)
{
    if (!transform) return false;

    auto computeFromLocal = [&](const AABB& localBounds)
    {
        instance.worldAABB.extents.x = std::abs(transform->scale.x) * localBounds.extents.x;
        instance.worldAABB.extents.y = std::abs(transform->scale.y) * localBounds.extents.y;
        instance.worldAABB.extents.z = std::abs(transform->scale.z) * localBounds.extents.z;

        instance.worldAABB.center.x = transform->position.x + transform->scale.x * localBounds.center.x;
        instance.worldAABB.center.y = transform->position.y + transform->scale.y * localBounds.center.y;
        instance.worldAABB.center.z = transform->position.z + transform->scale.z * localBounds.center.z;
    };

    if (m_componentManager.HasComponent<ColliderComponent>(entity))
    {
        auto& collider = m_componentManager.GetComponent<ColliderComponent>(entity);
        if (collider.enabled) {
            computeFromLocal(collider.localAABB);
            return true;
        }
    }

    if (m_componentManager.HasComponent<RenderComponent>(entity))
    {
        auto& render = m_componentManager.GetComponent<RenderComponent>(entity);
        if (render.mesh) {
            computeFromLocal(render.mesh->GetLocalBounds());
            return true;
        }
    }

    return false;
}

void RenderSystem::OnComponentAdded(Entity entity) {
    LOG_INFO("RenderSystem: Component Added to Entity " + std::to_string(entity.id) + ". Rebuilding Cache...");
    RebuildRenderCache();
}

void RenderSystem::OnComponentRemoved(Entity entity) {
    RebuildRenderCache();
}

} // namespace ECS
