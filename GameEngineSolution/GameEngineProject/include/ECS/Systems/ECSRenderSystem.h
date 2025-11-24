#pragma once

#include "../ComponentManager.h"
#include "../../Renderer/Renderer.h"
#include "../../EntityComponentSystem/Camera.h"

namespace ECS {

// ========================================
// RenderSystem
// Handles rendering for entities with
// RenderComponent + TransformComponent
// ========================================
class RenderSystem {
public:
    RenderSystem() = default;
    
    // Render all renderable entities
    void Render(ComponentManager& cm, Renderer* renderer, Camera& camera);
    
    // Debug rendering (bounding boxes)
    void RenderDebug(ComponentManager& cm, Renderer* renderer, Camera& camera);
};

} // namespace ECS
