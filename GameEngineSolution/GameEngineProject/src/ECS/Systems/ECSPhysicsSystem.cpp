#include "../../../include/ECS/Systems/ECSPhysicsSystem.h"
#include "../../../include/Physics/PhysicsConstants.h"
#include <algorithm>

using namespace PhysicsConstants;

namespace ECS {

void PhysicsSystem::Update(ComponentManager& cm, float deltaTime) {
    // Clamp deltaTime for safety
    if (deltaTime < MIN_DELTA_TIME) deltaTime = MIN_DELTA_TIME;
    if (deltaTime > MAX_DELTA_TIME) deltaTime = MAX_DELTA_TIME;
    
    // Get all entities with both Physics and Transform
    std::vector<Entity> entities = cm.GetEntitiesWithPhysicsAndTransform();
    
    for (Entity entity : entities) {
        PhysicsComponent* physics = cm.GetPhysics(entity);
        TransformComponent* transform = cm.GetTransform(entity);
        
        if (!physics || !transform) continue;
        
        // Apply physics forces
        if (physics->useGravity) {
            ApplyGravity(*physics, deltaTime);
        }
        
        ApplyDrag(*physics, deltaTime);
        ClampVelocity(*physics);
        
        // Integrate velocity into position
        IntegrateVelocity(*transform, *physics, deltaTime);
        
        // Simple ground collision
        if (physics->checkCollisions) {
            CheckGroundCollision(entity, *transform, *physics, cm);
        }
    }
}

void PhysicsSystem::ApplyGravity(PhysicsComponent& physics, float dt) {
    physics.velocity.y += physics.gravityAcceleration * dt;
}

void PhysicsSystem::ApplyDrag(PhysicsComponent& physics, float dt) {
    float dragFactor = 1.0f - (physics.drag * dt);
    if (dragFactor < 0.0f) dragFactor = 0.0f;
    
    physics.velocity.x *= dragFactor;
    physics.velocity.z *= dragFactor;
}

void PhysicsSystem::ClampVelocity(PhysicsComponent& physics) {
    if (physics.velocity.y < physics.maxFallSpeed) {
        physics.velocity.y = physics.maxFallSpeed;
    }
}

void PhysicsSystem::IntegrateVelocity(TransformComponent& transform, PhysicsComponent& physics, float dt) {
    transform.position.x += physics.velocity.x * dt;
    transform.position.y += physics.velocity.y * dt;
    transform.position.z += physics.velocity.z * dt;
}

void PhysicsSystem::CheckGroundCollision(Entity entity, TransformComponent& transform, PhysicsComponent& physics, ComponentManager& cm) {
    // Simple ground collision
    // Floor is at Y = -1.0 with scale Y = 0.1 (half-extent 0.05) -> Top surface at -0.95
    const float GROUND_Y = -0.95f; 
    
    float entityHalfHeight = 0.5f; // Default fallback
    
    // Try to get collider for accurate bounds
    if (cm.HasCollider(entity)) {
        ColliderComponent* collider = cm.GetCollider(entity);
        if (collider) {
            entityHalfHeight = collider->localAABB.extents.y * transform.scale.y;
        }
    }
    
    float bottomY = transform.position.y - entityHalfHeight;
    
    if (bottomY <= GROUND_Y) {
        // Collision with ground
        transform.position.y = GROUND_Y + entityHalfHeight;
        physics.velocity.y = 0.0f;
        physics.isGrounded = true;
    } else {
        physics.isGrounded = false;
    }
    
    // TODO: Full collision detection with other entities using ColliderComponent
    // This would iterate through all entities with colliders and check AABB intersections
}

} // namespace ECS
