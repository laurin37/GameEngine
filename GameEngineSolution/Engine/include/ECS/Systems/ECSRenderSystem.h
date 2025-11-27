#pragma once

#include "../ComponentManager.h"
#include "../System.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/Camera.h"

namespace ECS {

// ========================================
// RenderSystem
// Handles rendering for entities with
// RenderComponent + TransformComponent
// ========================================
class RenderSystem : public System {
public:
    explicit RenderSystem(ComponentManager& cm) : System(cm) {}
    
    // Render all renderable entities
    void Render(Renderer* renderer, Camera& camera, const struct DirectionalLight& dirLight);
    
    // Debug rendering (bounding boxes)
    void RenderDebug(Renderer* renderer, Camera& camera);
    
    // Lifecycle
    void Init() override;
    void Shutdown() override;
    
    // Update cache (called by SystemManager)
    void Update(float deltaTime) override;
    
    // Event handlers
    void OnComponentAdded(Entity entity);
    void OnComponentRemoved(Entity entity);
    
    // Force cache rebuild (useful after scene load)
    void RebuildRenderCache();

private:
    void UpdateRenderCache();
    void CreateRenderCacheEntry(Entity entity, const TransformComponent* transform, const RenderComponent* render);
    void RemoveRenderCacheEntry(size_t index);
    void RefreshRenderCacheEntry(size_t index, const TransformComponent* transform, const RenderComponent* render);
    bool TryComputeWorldBounds(Entity entity, const TransformComponent* transform, Renderer::RenderInstance& instance);

    struct RenderCacheEntry
    {
        Entity entity = NULL_ENTITY;
        Renderer::RenderInstance instance{};
        DirectX::XMFLOAT3 lastPosition{ 0.0f,0.0f,0.0f };
        DirectX::XMFLOAT3 lastRotation{ 0.0f,0.0f,0.0f };
        DirectX::XMFLOAT3 lastScale{ 1.0f,1.0f,1.0f };
    };

    std::vector<RenderCacheEntry> m_renderCache;
    std::unordered_map<Entity, size_t> m_entityToRenderCacheIndex;
    std::vector<int> m_eventSubscriptions; // Using int for SubscriptionId (it's a typedef)
};

} // namespace ECS
