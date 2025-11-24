#pragma once

#include <DirectXMath.h>
#include <memory>
#include "../Physics/Collision.h"

// Forward declarations
class Mesh;
class Material;

namespace ECS {

// ========================================
// Transform Component
// Position, rotation, and scale in 3D space
// ========================================
struct TransformComponent {
    DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };  // Euler angles (radians)
    DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
};

// ========================================
// Physics Component
// Velocity, forces, and physics properties
// ========================================
struct PhysicsComponent {
    DirectX::XMFLOAT3 velocity = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 acceleration = { 0.0f, 0.0f, 0.0f };
    
    float mass = 1.0f;
    float drag = 0.0f;
    float gravityAcceleration = -15.0f;
    float maxFallSpeed = -15.0f;
    
    bool useGravity = true;
    bool checkCollisions = true;
    bool isGrounded = false;
};

// ========================================
// Render Component
// Mesh and material for rendering
// ========================================
struct RenderComponent {
    Mesh* mesh = nullptr;
    std::shared_ptr<Material> material;
};

// ========================================
// Collider Component
// Collision detection volume
// ========================================
struct ColliderComponent {
    AABB localAABB;     // Bounding box in local space
    bool enabled = true;
};

} // namespace ECS
