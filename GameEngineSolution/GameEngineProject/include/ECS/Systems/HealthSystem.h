#pragma once

#include "../ComponentManager.h"
#include "../System.h"

class HealthSystem : public ECS::System {
public:
    explicit HealthSystem(ECS::ComponentManager& cm) : ECS::System(cm) {}
    void Update(float deltaTime) override;
};
