#define NOMINMAX
#include "../../../include/ECS/Systems/WeaponSystem.h"
#include <iostream>
#include <format>
#include <cmath>
#include <algorithm> // For std::max

void WeaponSystem::Update(ECS::ComponentManager& componentManager, Input& input, float deltaTime) {
    auto weaponArray = componentManager.GetComponentArray<ECS::WeaponComponent>();
    
    for (size_t i = 0; i < weaponArray->GetSize(); ++i) {
        ECS::Entity entity = weaponArray->GetEntityAtIndex(i);
        ECS::WeaponComponent& weapon = weaponArray->GetData(entity);

        // Cooldown management
        if (weapon.timeSinceLastShot < weapon.fireRate) {
            weapon.timeSinceLastShot += deltaTime;
        }

        // Check if this entity is controlled by player (has PlayerController)
        // Only player weapons respond to input for now
        if (componentManager.HasComponent<ECS::PlayerControllerComponent>(entity)) {
            // TODO: Implement proper mouse button support in Input class
            // For now, use VK_LBUTTON for both. 
            // Note: IsKeyDown is "held down", so semi-auto might fire repeatedly if fireRate is low.
            bool fireInput = input.IsKeyDown(VK_LBUTTON);
            
            if (fireInput && weapon.timeSinceLastShot >= weapon.fireRate && weapon.currentAmmo > 0) {
                if (componentManager.HasComponent<ECS::TransformComponent>(entity)) {
                    ECS::TransformComponent& transform = componentManager.GetComponent<ECS::TransformComponent>(entity);
                    FireWeapon(entity, weapon, transform, componentManager);
                }
            }
        }
    }
}

void WeaponSystem::FireWeapon(ECS::Entity entity, ECS::WeaponComponent& weapon, ECS::TransformComponent& transform, ECS::ComponentManager& componentManager) {
    weapon.timeSinceLastShot = 0.0f;
    weapon.currentAmmo--;

    std::cout << std::format("Bang! Ammo: {}/{}", weapon.currentAmmo, weapon.maxAmmo) << std::endl;

    // Calculate ray origin and direction
    // Origin is player position + camera offset (if any)
    DirectX::XMFLOAT3 rayOrigin = transform.position;
    
    // Adjust for camera height if available
    if (componentManager.HasComponent<ECS::PlayerControllerComponent>(entity)) {
        auto& pc = componentManager.GetComponent<ECS::PlayerControllerComponent>(entity);
        rayOrigin.y += pc.cameraHeight;
    }

    // Direction based on rotation (Pitch/Yaw)
    // Assuming rotation.x is pitch, rotation.y is yaw
    float pitch = transform.rotation.x;
    float yaw = transform.rotation.y;
    
    // If player controller exists, use its view pitch
    if (componentManager.HasComponent<ECS::PlayerControllerComponent>(entity)) {
        pitch = componentManager.GetComponent<ECS::PlayerControllerComponent>(entity).viewPitch;
    }

    DirectX::XMFLOAT3 rayDir;
    rayDir.x = cos(pitch) * sin(yaw);
    rayDir.y = sin(pitch);
    rayDir.z = cos(pitch) * cos(yaw);

    // Normalize direction
    float length = sqrt(rayDir.x * rayDir.x + rayDir.y * rayDir.y + rayDir.z * rayDir.z);
    rayDir.x /= length;
    rayDir.y /= length;
    rayDir.z /= length;

    // Raycast against all entities with Health and Collider
    // This is a naive O(N) raycast. In a real engine, use a spatial partition (Octree/BVH).
    
    ECS::Entity hitEntity = ECS::NULL_ENTITY;
    float minDistance = weapon.range;

    auto colliderArray = componentManager.GetComponentArray<ECS::ColliderComponent>();
    for (size_t i = 0; i < colliderArray->GetSize(); ++i) {
        ECS::Entity targetEntity = colliderArray->GetEntityAtIndex(i);
        if (targetEntity == entity) continue; // Don't hit self

        if (!componentManager.HasComponent<ECS::HealthComponent>(targetEntity)) continue;
        if (!componentManager.HasComponent<ECS::TransformComponent>(targetEntity)) continue;

        auto& targetCollider = colliderArray->GetData(targetEntity);
        if (!targetCollider.enabled) continue;

        auto& targetTransform = componentManager.GetComponent<ECS::TransformComponent>(targetEntity);

        // Approximate collider as a sphere for simple raycasting
        // Use the largest extent of the AABB * scale
        float radius = std::max({
            targetCollider.localAABB.extents.x * targetTransform.scale.x,
            targetCollider.localAABB.extents.y * targetTransform.scale.y,
            targetCollider.localAABB.extents.z * targetTransform.scale.z
        });

        // Center of the sphere is transform position + rotated local center (ignoring rotation for simplicity here)
        DirectX::XMFLOAT3 center = {
            targetTransform.position.x + targetCollider.localAABB.center.x * targetTransform.scale.x,
            targetTransform.position.y + targetCollider.localAABB.center.y * targetTransform.scale.y,
            targetTransform.position.z + targetCollider.localAABB.center.z * targetTransform.scale.z
        };

        float t = 0.0f;
        if (RaySphereIntersect(rayOrigin, rayDir, center, radius, t)) {
            if (t < minDistance) {
                minDistance = t;
                hitEntity = targetEntity;
            }
        }
    }

    if (hitEntity != ECS::NULL_ENTITY) {
        std::cout << std::format("Hit Entity {}!", hitEntity) << std::endl;
        
        auto& health = componentManager.GetComponent<ECS::HealthComponent>(hitEntity);
        health.currentHealth -= weapon.damage;
        
        std::cout << std::format("Entity {} Health: {}", hitEntity, health.currentHealth) << std::endl;
    }
}

bool WeaponSystem::RaySphereIntersect(
    const DirectX::XMFLOAT3& rayOrigin, 
    const DirectX::XMFLOAT3& rayDir, 
    const DirectX::XMFLOAT3& sphereCenter, 
    float sphereRadius, 
    float& t
) {
    DirectX::XMFLOAT3 m = {
        rayOrigin.x - sphereCenter.x,
        rayOrigin.y - sphereCenter.y,
        rayOrigin.z - sphereCenter.z
    };

    float b = m.x * rayDir.x + m.y * rayDir.y + m.z * rayDir.z;
    float c = (m.x * m.x + m.y * m.y + m.z * m.z) - sphereRadius * sphereRadius;

    // Exit if ray's origin is outside sphere (c > 0) and ray is pointing away from sphere (b > 0)
    if (c > 0.0f && b > 0.0f) return false;

    float discr = b * b - c;

    // A negative discriminant corresponds to ray missing sphere
    if (discr < 0.0f) return false;

    // Ray now found to intersect sphere, compute smallest t value of intersection
    t = -b - sqrt(discr);

    // If t is negative, ray started inside sphere so clamp t to zero
    if (t < 0.0f) t = 0.0f;

    return true;
}
