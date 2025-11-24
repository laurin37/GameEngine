#pragma once

#include "../ComponentManager.h"

namespace ECS {

// ========================================
// PhysicsSystem
// Handles physics simulation for entities
// with PhysicsComponent + TransformComponent
// ========================================
class PhysicsSystem {
public:
    PhysicsSystem() = default;
    
    // Update all physics entities
    void Update(ComponentManager& cm, float deltaTime);
    
private:
    // Physics sub-steps
    void ApplyGravity(PhysicsComponent& physics, float dt);
    void ApplyDrag(PhysicsComponent& physics, float dt);
    void ClampVelocity(PhysicsComponent& physics);
    void IntegrateVelocity(TransformComponent& transform, PhysicsComponent& physics, float dt);
    
    // Collision (simplified - no GameObject dependency)
    void CheckGroundCollision(Entity entity, TransformComponent& transform, PhysicsComponent& physics, ComponentManager& cm);
    
    // Physics constants
    static constexpr float MIN_DELTA_TIME = 0.0001f;
    static constexpr float MAX_DELTA_TIME = 0.1f;
};

} // namespace ECS
