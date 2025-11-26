#include "Systems/PlayerMovementSystem.h"
#include "Input/Input.h"
#include "Events/InputEvents.h"
#include "Events/EventBus.h"
#include <DirectXMath.h>
#include "Utils/Logger.h"

using namespace DirectX;

namespace ECS {

void PlayerMovementSystem::Init() {
    // Cache component arrays for performance
    m_controllerArray = m_componentManager.GetComponentArray<PlayerControllerComponent>();
    m_transformArray = m_componentManager.GetComponentArray<TransformComponent>();
    m_physicsArray = m_componentManager.GetComponentArray<PhysicsComponent>();
    
    // Subscribe to keyboard events (EventBus set by SystemManager)
    if (m_eventBus) {
        m_eventBus->Subscribe(EventType::KeyPressed, [this](Event& e) {
            this->OnEvent(e);
        });
    }
}

void PlayerMovementSystem::Update(float deltaTime) {
    // Iterate over all player controller components (cached array)
    auto& controllerVec = m_controllerArray->GetComponentArray();
    
    for (size_t i = 0; i < controllerVec.size(); ++i) {
        Entity entity = m_controllerArray->GetEntityAtIndex(i);
        PlayerControllerComponent& controller = controllerVec[i];
        
        if (!m_componentManager.HasComponent<TransformComponent>(entity)) continue;
        TransformComponent& transform = m_componentManager.GetComponent<TransformComponent>(entity);
        
        // Optional physics
        PhysicsComponent* physics = m_componentManager.GetComponentPtr<PhysicsComponent>(entity);
        
        // Handle mouse look and camera
        HandleMouseLook(entity, transform, controller, m_input, deltaTime);
        
        // Handle movement (WASD)
        if (physics) {
            HandleMovement(entity, transform, *physics, controller, m_input, deltaTime);
        }
    }
}

void PlayerMovementSystem::HandleMovement(Entity entity, TransformComponent& transform, PhysicsComponent& physics,
                                         PlayerControllerComponent& controller, Input& input, float deltaTime) {
    // Get movement direction from input
    XMFLOAT3 moveDir = { 0.0f, 0.0f, 0.0f };
    
    if (input.IsActionDown(Action::MoveForward)) moveDir.z += 1.0f;
    if (input.IsActionDown(Action::MoveBackward)) moveDir.z -= 1.0f;
    if (input.IsActionDown(Action::MoveRight)) moveDir.x += 1.0f;
    if (input.IsActionDown(Action::MoveLeft)) moveDir.x -= 1.0f;
    
    // Normalize and apply speed
    XMVECTOR moveDirVec = XMLoadFloat3(&moveDir);
    float length = XMVectorGetX(XMVector3Length(moveDirVec));
    
    if (length > 0.0f) {
        moveDirVec = XMVector3Normalize(moveDirVec);
        
        // Rotate movement direction based on player's Y rotation (yaw)
        float yaw = transform.rotation.y;
        XMMATRIX rotMatrix = XMMatrixRotationY(yaw);
        moveDirVec = XMVector3Transform(moveDirVec, rotMatrix);
        
        // Apply horizontal movement (preserve vertical velocity)
        XMFLOAT3 finalMove;
        XMStoreFloat3(&finalMove, moveDirVec);
        
        physics.velocity.x = finalMove.x * controller.moveSpeed;
        physics.velocity.z = finalMove.z * controller.moveSpeed;
    } else {
        // No input, stop horizontal movement
        physics.velocity.x = 0.0f;
        physics.velocity.z = 0.0f;
    }
}

void PlayerMovementSystem::HandleMouseLook(Entity entity, TransformComponent& transform,
                                          PlayerControllerComponent& controller, 
                                          Input& input, float deltaTime) {
    // Get mouse delta
    float mouseDeltaX = static_cast<float>(input.GetMouseDeltaX());
    float mouseDeltaY = static_cast<float>(input.GetMouseDeltaY());
    
    // Apply rotation (yaw = Y rotation, pitch = X rotation)
    transform.rotation.y += mouseDeltaX * controller.mouseSensitivity;  // Yaw
    controller.viewPitch += mouseDeltaY * controller.mouseSensitivity;  // Pitch
    
    // Clamp pitch to prevent flipping
    const float maxPitch = DirectX::XM_PI / 2.0f - 0.1f;
    if (controller.viewPitch > maxPitch) controller.viewPitch = maxPitch;
    if (controller.viewPitch < -maxPitch) controller.viewPitch = -maxPitch;

    // Keep the physical player mesh upright (only yaw affects it)
    transform.rotation.x = 0.0f;
    transform.rotation.z = 0.0f;

    // Note: CameraSystem will handle updating the camera view matrix based on this transform
}

void PlayerMovementSystem::OnEvent(Event& e)
{
    // Only handle KeyPressed events
    if (e.GetEventType() != EventType::KeyPressed) return;
    
    KeyPressedEvent& event = static_cast<KeyPressedEvent&>(e);
    
    if (event.GetKeyCode() == VK_SPACE)
    {
        // Iterate over all player controller components (cached array)
        auto& controllerVec = m_controllerArray->GetComponentArray();

        for (size_t i = 0; i < controllerVec.size(); ++i) {
            Entity entity = m_controllerArray->GetEntityAtIndex(i);
            PlayerControllerComponent& controller = controllerVec[i];

            if (!m_componentManager.HasComponent<PhysicsComponent>(entity)) continue;
            PhysicsComponent& physics = m_componentManager.GetComponent<PhysicsComponent>(entity);

            // Jump logic
            if (physics.isGrounded && controller.canJump) {
                physics.velocity.y = controller.jumpForce;
                e.Handled = true;
            }
        }
    }
}

} // namespace ECS

