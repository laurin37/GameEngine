#include "../../../include/ECS/Systems/ECSPhysicsSystem.h"
#include "../../../include/Physics/PhysicsConstants.h"
#include <algorithm>

using namespace PhysicsConstants;

namespace ECS {

void PhysicsSystem::Init() {
    // Cache component arrays for performance
    m_physicsArray = m_componentManager.GetComponentArray<PhysicsComponent>();
    m_transformArray = m_componentManager.GetComponentArray<TransformComponent>();
    m_colliderArray = m_componentManager.GetComponentArray<ColliderComponent>();
}

void PhysicsSystem::Update(float deltaTime) {
    // Clamp deltaTime for safety
    if (deltaTime < MIN_DELTA_TIME) deltaTime = MIN_DELTA_TIME;
    if (deltaTime > MAX_DELTA_TIME) deltaTime = MAX_DELTA_TIME;
    
    // Rebuild spatial grid for this frame
    RebuildSpatialGrid();
    
    // Use Query API to get entities with both Physics and Transform
    auto entities = m_componentManager.QueryEntities<PhysicsComponent, TransformComponent>();
    
    for (Entity entity : entities) {
        auto& physics = m_componentManager.GetComponent<PhysicsComponent>(entity);
        auto& transform = m_componentManager.GetComponent<TransformComponent>(entity);
        
        // Apply physics forces
        if (physics.useGravity) {
            ApplyGravity(physics, deltaTime);
        }
        
        ApplyDrag(physics, deltaTime);
        ClampVelocity(physics);
        
        // Integrate velocity into position
        IntegrateVelocity(transform, physics, deltaTime);
        
        // Simple ground collision
        if (physics.checkCollisions) {
            CheckGroundCollision(entity, transform, physics);
        }
    }
}

void PhysicsSystem::RebuildSpatialGrid() {
    // Clear previous frame's data
    m_spatialGrid.Clear();
    
    // Insert all colliders into the grid
    auto entities = m_componentManager.QueryEntities<ColliderComponent, TransformComponent>();
    
    for (Entity entity : entities) {
        const auto& collider = m_componentManager.GetComponent<ColliderComponent>(entity);
        
        if (!collider.enabled) continue;
        
        const auto& transform = m_componentManager.GetComponent<TransformComponent>(entity);
        
        // Calculate world-space AABB
        AABB worldAABB;
        worldAABB.extents = {
            collider.localAABB.extents.x * transform.scale.x,
            collider.localAABB.extents.y * transform.scale.y,
            collider.localAABB.extents.z * transform.scale.z
        };
        
        float centerOffsetY = collider.localAABB.center.y * transform.scale.y;
        worldAABB.center = {
            transform.position.x,
            transform.position.y + centerOffsetY,
            transform.position.z
        };
        
        // Insert into spatial grid
        m_spatialGrid.Insert(entity, worldAABB);
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

void PhysicsSystem::CheckGroundCollision(Entity entity, TransformComponent& transform, PhysicsComponent& physics) {
    // Full collision detection using spatial grid
    if (!m_componentManager.HasComponent<ColliderComponent>(entity)) return;
    
    ColliderComponent& myCollider = m_componentManager.GetComponent<ColliderComponent>(entity);
    if (!myCollider.enabled) return;
    
    // Reset grounded state
    physics.isGrounded = false;
    
    // Calculate my world-space AABB
    DirectX::XMFLOAT3 myExtents = {
        myCollider.localAABB.extents.x * transform.scale.x,
        myCollider.localAABB.extents.y * transform.scale.y,
        myCollider.localAABB.extents.z * transform.scale.z
    };
    
    float colliderCenterOffsetY = myCollider.localAABB.center.y * transform.scale.y;
    float colliderCenterY = transform.position.y + colliderCenterOffsetY;
    
    DirectX::XMFLOAT3 myMin, myMax;
    myMin.x = transform.position.x - myExtents.x;
    myMin.y = colliderCenterY - myExtents.y;
    myMin.z = transform.position.z - myExtents.z;
    myMax.x = transform.position.x + myExtents.x;
    myMax.y = colliderCenterY + myExtents.y;
    myMax.z = transform.position.z + myExtents.z;
    
    // Query spatial grid for nearby entities (HUGE performance improvement!)
    AABB queryBounds;
    queryBounds.center = { transform.position.x, colliderCenterY, transform.position.z };
    queryBounds.extents = myExtents;
    
    std::vector<Entity> nearbyEntities = m_spatialGrid.Query(queryBounds);
    
    // Check collisions only with nearby entities
    for (Entity other : nearbyEntities) {
        if (other == entity) continue; // Skip self
        
        if (!m_componentManager.HasComponent<ColliderComponent>(other)) continue;
        const ColliderComponent& otherCollider = m_componentManager.GetComponent<ColliderComponent>(other);
        
        if (!otherCollider.enabled) continue;
        if (!m_componentManager.HasComponent<TransformComponent>(other)) continue;
        
        const TransformComponent& otherTransform = m_componentManager.GetComponent<TransformComponent>(other);
        
        // Calculate other's world-space AABB
        DirectX::XMFLOAT3 otherExtents = {
            otherCollider.localAABB.extents.x * otherTransform.scale.x,
            otherCollider.localAABB.extents.y * otherTransform.scale.y,
            otherCollider.localAABB.extents.z * otherTransform.scale.z
        };
        
        float otherCenterOffsetY = otherCollider.localAABB.center.y * otherTransform.scale.y;
        float otherColliderCenterY = otherTransform.position.y + otherCenterOffsetY;
        
        DirectX::XMFLOAT3 otherMin, otherMax;
        otherMin.x = otherTransform.position.x - otherExtents.x;
        otherMin.y = otherColliderCenterY - otherExtents.y;
        otherMin.z = otherTransform.position.z - otherExtents.z;
        otherMax.x = otherTransform.position.x + otherExtents.x;
        otherMax.y = otherColliderCenterY + otherExtents.y;
        otherMax.z = otherTransform.position.z + otherExtents.z;
        
        // AABB intersection test
        bool intersects = 
            myMin.x <= otherMax.x && myMax.x >= otherMin.x &&
            myMin.y <= otherMax.y && myMax.y >= otherMin.y &&
            myMin.z <= otherMax.z && myMax.z >= otherMin.z;
        
        if (intersects) {
            // Calculate penetration depth on each axis
            float penetrationX = (std::min)(myMax.x - otherMin.x, otherMax.x - myMin.x);
            float penetrationY = (std::min)(myMax.y - otherMin.y, otherMax.y - myMin.y);
            float penetrationZ = (std::min)(myMax.z - otherMin.z, otherMax.z - myMin.z);
            
            // Resolve on axis with smallest penetration
            if (penetrationX < penetrationY && penetrationX < penetrationZ) {
                // Resolve on X axis
                if (transform.position.x < otherTransform.position.x) {
                    transform.position.x -= penetrationX;
                } else {
                    transform.position.x += penetrationX;
                }
                physics.velocity.x = 0.0f;
            } else if (penetrationY < penetrationZ) {
                // Resolve on Y axis
                if (colliderCenterY < otherColliderCenterY) {
                    // We're below the other object - push us down (hitting ceiling)
                    transform.position.y -= penetrationY;
                    physics.velocity.y = 0.0f;
                } else {
                    // We're above the other object - push us up (standing on floor)
                    transform.position.y += penetrationY;
                    physics.velocity.y = 0.0f;
                    physics.isGrounded = true; // We're standing on something!
                }
            } else {
                // Resolve on Z axis
                if (transform.position.z < otherTransform.position.z) {
                    transform.position.z -= penetrationZ;
                } else {
                    transform.position.z += penetrationZ;
                }
                physics.velocity.z = 0.0f;
            }
        }
    }
}

} // namespace ECS

