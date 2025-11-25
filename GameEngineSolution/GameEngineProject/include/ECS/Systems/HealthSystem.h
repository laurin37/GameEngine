#pragma once

#include "../ComponentManager.h"

class HealthSystem {
public:
    void Update(ECS::ComponentManager& componentManager, float deltaTime);
};
