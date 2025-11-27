#pragma once

#include "../ComponentManager.h"
#include "../System.h"
#include "../SystemPhase.h"
#include "../../Physics/SpatialGrid.h"
#include <memory>

namespace ECS {

// ==================================================================================
// PhysicsSystem
// ----------------------------------------------------------------------------------
// Handles physics simulation for entities with PhysicsComponent + TransformComponent
// 
// Improvements:
// - Spatial grid for O(n·k) collision detection
// - Cached component arrays for performance
// - PostUpdate phase for physics integration
// - Can run in parallel (thread-safe reads, careful writes)
// ==================================================================================
class PhysicsSystem : public System {
public:
    explicit PhysicsSystem(ComponentManager& cm) : System(cm), m_spatialGrid(10.0f) {}
    
    // Lifecycle
    void Init() override;
    void Update(float deltaTime) override;
    
    // Phase and parallelization
    SystemPhase GetPhase() const override { return SystemPhase::PostUpdate; }
    bool CanParallelize() const override { return false; } // Writes to transforms
    
    // Expose spatial grid for other systems
    const Physics::SpatialGrid& GetSpatialGrid() const { return m_spatialGrid; }
    
private:
    // Physics sub-steps
    void ApplyGravity(PhysicsComponent& physics, float dt);
    void ApplyDrag(PhysicsComponent& physics, float dt);
    void ClampVelocity(PhysicsComponent& physics);
    void IntegrateVelocity(TransformComponent& transform, PhysicsComponent& physics, float dt);
    
    // Collision detection
    void RebuildSpatialGrid();
    void CheckGroundCollision(Entity entity, TransformComponent& transform, PhysicsComponent& physics);
    
    // Cached component arrays
    std::shared_ptr<ComponentArray<PhysicsComponent>> m_physicsArray;
    std::shared_ptr<ComponentArray<TransformComponent>> m_transformArray;
    std::shared_ptr<ComponentArray<ColliderComponent>> m_colliderArray;
    
    // Spatial partitioning for collision
    Physics::SpatialGrid m_spatialGrid;
    
    // Physics constants
    static constexpr float MIN_DELTA_TIME = 0.0001f;
    static constexpr float MAX_DELTA_TIME = 0.1f;
};

} // namespace ECS

