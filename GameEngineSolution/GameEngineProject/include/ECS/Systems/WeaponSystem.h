#pragma once

#include "../ComponentManager.h"
#include "../../Input/Input.h"

class WeaponSystem {
public:
    void Update(ECS::ComponentManager& componentManager, Input& input, float deltaTime);

private:
    void FireWeapon(ECS::Entity entity, ECS::WeaponComponent& weapon, ECS::TransformComponent& transform, ECS::ComponentManager& componentManager);
    
    // Simple ray-sphere intersection for hit detection
    bool RaySphereIntersect(
        const DirectX::XMFLOAT3& rayOrigin, 
        const DirectX::XMFLOAT3& rayDir, 
        const DirectX::XMFLOAT3& sphereCenter, 
        float sphereRadius, 
        float& t
    );
};
