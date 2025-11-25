#include "../../../include/ECS/Systems/ProjectileSystem.h"
#include <iostream>
#include <format>

void ProjectileSystem::Update(ECS::ComponentManager& componentManager, float deltaTime) {
    auto projectileArray = componentManager.GetComponentArray<ECS::ProjectileComponent>();
    
    // Iterate backwards to safely remove entities
    for (int i = (int)projectileArray->GetSize() - 1; i >= 0; --i) {
        ECS::Entity entity = projectileArray->GetEntityAtIndex(i);
        ECS::ProjectileComponent& projectile = projectileArray->GetData(entity);

        // Update lifetime
        projectile.lifetime -= deltaTime;
        if (projectile.lifetime <= 0.0f) {
            componentManager.DestroyEntity(entity);
            continue;
        }

        // Update position based on velocity
        if (componentManager.HasComponent<ECS::TransformComponent>(entity)) {
            ECS::TransformComponent& transform = componentManager.GetComponent<ECS::TransformComponent>(entity);
            
            transform.position.x += projectile.velocity.x * projectile.speed * deltaTime;
            transform.position.y += projectile.velocity.y * projectile.speed * deltaTime;
            transform.position.z += projectile.velocity.z * projectile.speed * deltaTime;
        }

        // Simple collision check (Point vs AABB)
        // In a real engine, use the PhysicsSystem for this
        auto colliderArray = componentManager.GetComponentArray<ECS::ColliderComponent>();
        for (size_t j = 0; j < colliderArray->GetSize(); ++j) {
            ECS::Entity targetEntity = colliderArray->GetEntityAtIndex(j);
            if (targetEntity == entity) continue; // Don't hit self

            if (!componentManager.HasComponent<ECS::TransformComponent>(targetEntity)) continue;
            if (!componentManager.HasComponent<ECS::HealthComponent>(targetEntity)) continue;

            auto& targetCollider = colliderArray->GetData(targetEntity);
            if (!targetCollider.enabled) continue;

            auto& targetTransform = componentManager.GetComponent<ECS::TransformComponent>(targetEntity);
            
            // Check if projectile is inside target AABB
            // Transform projectile position to local space of target? 
            // Or transform AABB to world space? Let's do world space AABB approximation.
            
            if (componentManager.HasComponent<ECS::TransformComponent>(entity)) {
                auto& projTransform = componentManager.GetComponent<ECS::TransformComponent>(entity);
                
                float minX = targetTransform.position.x + (targetCollider.localAABB.center.x - targetCollider.localAABB.extents.x) * targetTransform.scale.x;
                float maxX = targetTransform.position.x + (targetCollider.localAABB.center.x + targetCollider.localAABB.extents.x) * targetTransform.scale.x;
                float minY = targetTransform.position.y + (targetCollider.localAABB.center.y - targetCollider.localAABB.extents.y) * targetTransform.scale.y;
                float maxY = targetTransform.position.y + (targetCollider.localAABB.center.y + targetCollider.localAABB.extents.y) * targetTransform.scale.y;
                float minZ = targetTransform.position.z + (targetCollider.localAABB.center.z - targetCollider.localAABB.extents.z) * targetTransform.scale.z;
                float maxZ = targetTransform.position.z + (targetCollider.localAABB.center.z + targetCollider.localAABB.extents.z) * targetTransform.scale.z;

                if (projTransform.position.x >= minX && projTransform.position.x <= maxX &&
                    projTransform.position.y >= minY && projTransform.position.y <= maxY &&
                    projTransform.position.z >= minZ && projTransform.position.z <= maxZ) {
                    
                    // Hit!
                    std::cout << std::format("Projectile hit Entity {}!", targetEntity) << std::endl;
                    
                    auto& health = componentManager.GetComponent<ECS::HealthComponent>(targetEntity);
                    health.currentHealth -= projectile.damage;
                    
                    componentManager.DestroyEntity(entity); // Destroy projectile
                    break; // Stop checking collisions for this projectile
                }
            }
        }
    }
}
