#pragma once

#include "../ComponentManager.h"

class ProjectileSystem {
public:
    void Update(ECS::ComponentManager& componentManager, float deltaTime);
};
