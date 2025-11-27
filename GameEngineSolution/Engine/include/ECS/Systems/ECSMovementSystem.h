#pragma once

#include "../ComponentManager.h"
#include "../System.h"

namespace ECS {

class MovementSystem : public System {
public:
    explicit MovementSystem(ComponentManager& cm) : System(cm) {}
    void Update(float deltaTime) override;
    
    // Enable parallel execution
    bool CanParallelize() const override { return true; }
};

} // namespace ECS
