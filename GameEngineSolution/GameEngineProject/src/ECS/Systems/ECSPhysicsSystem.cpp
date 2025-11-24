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
            CheckGroundCollision(*transform, *physics, cm);
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

void PhysicsSystem::CheckGroundCollision(TransformComponent& transform, PhysicsComponent& physics, ComponentManager& cm) {
    // Simple ground plane at Y=0
    const float GROUND_Y = 0.0f;
    const float ENTITY_HALF_HEIGHT = 0.5f;  // Assume 1 unit tall entity
    
    float bottomY = transform.position.y - ENTITY_HALF_HEIGHT;
    
    if (bottomY <= GROUND_Y) {
        // Collision with ground
        transform.position.y = GROUND_Y + ENTITY_HALF_HEIGHT;
        physics.velocity.y = 0.0f;
        physics.isGrounded = true;
    } else {
        physics.isGrounded = false;
    }
    
    // TODO: Full collision detection with other entities using ColliderComponent
    // This would iterate through all entities with colliders and check AABB intersections
}

} // namespace ECS
