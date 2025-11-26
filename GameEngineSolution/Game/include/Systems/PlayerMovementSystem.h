#pragma once

#include "ECS/ComponentManager.h"
#include "ECS/System.h"
#include "ECS/SystemPhase.h"
#include "Events/Event.h"
#include <memory>

// Forward declarations
class Input;

namespace ECS {

// ==================================================================================
// PlayerMovementSystem
// ----------------------------------------------------------------------------------
// Handles first-person player movement and camera control
// - WASD movement (continuous input)
// - Mouse look (continuous input)
// - Jump (event-based input via EventBus)
// ==================================================================================
class PlayerMovementSystem : public System {
public:
    PlayerMovementSystem(ComponentManager& cm, Input& input) 
        : System(cm), m_input(input) {}

    void Init() override;
    void Update(float deltaTime) override;
    void OnEvent(Event& e);
    
    // System phase: PreUpdate (before physics)
    SystemPhase GetPhase() const override { return SystemPhase::PreUpdate; }
    bool CanParallelize() const override { return false; } // Writes to physics/transform

private:
    Input& m_input;
    
    // Cached component arrays
    std::shared_ptr<ComponentArray<PlayerControllerComponent>> m_controllerArray;
    std::shared_ptr<ComponentArray<TransformComponent>> m_transformArray;
    std::shared_ptr<ComponentArray<PhysicsComponent>> m_physicsArray;
    
    void HandleMovement(Entity entity, TransformComponent& transform, PhysicsComponent& physics, 
                       PlayerControllerComponent& controller, Input& input, float deltaTime);
    void HandleMouseLook(Entity entity, TransformComponent& transform, PlayerControllerComponent& controller, 
                        Input& input, float deltaTime);
};

} // namespace ECS

