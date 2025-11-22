#include "include/Player.h"
#include "include/PhysicsSystem.h" 

using namespace DirectX;

Player::Player(Mesh* mesh, std::shared_ptr<Material> material, Camera* camera)
    : GameObject(mesh, material), m_camera(camera), m_velocity({ 0,0,0 }), m_onGround(false)
{
    // Set a smaller bounding box for the player (human size)
    AABB playerBox;
    playerBox.center = { 0, 1.0f, 0 };
    playerBox.extents = { 0.4f, 0.9f, 0.4f };
    SetBoundingBox(playerBox);
}

void Player::Update(float deltaTime, Input& input, const std::vector<std::unique_ptr<GameObject>>& worldObjects)
{
    // 1. Handle Rotation (Mouse Look)
    float rotSpeed = 0.5f * deltaTime;
    float mouseDx = static_cast<float>(input.GetMouseDeltaX()) * rotSpeed;
    float mouseDy = static_cast<float>(input.GetMouseDeltaY()) * rotSpeed;
    m_camera->AdjustRotation(mouseDy, mouseDx, 0.0f);

    // Sync Player Rotation with Camera Y-rotation 
    // FIX: GetRotation returns XMVECTOR, so we store it in a vector variable first.
    XMVECTOR camRotVec = m_camera->GetRotation();

    // Extract the Y component (Yaw) directly
    float camYaw = XMVectorGetY(camRotVec);

    SetRotation(0.0f, camYaw, 0.0f);

    // 2. Handle Movement Input (WASD)
    // We use camYaw instead of camRot.y
    XMVECTOR forward = XMVectorSet(sin(camYaw), 0.0f, cos(camYaw), 0.0f);
    XMVECTOR right = XMVectorSet(cos(camYaw), 0.0f, -sin(camYaw), 0.0f);

    XMVECTOR moveDir = XMVectorZero();

    if (input.IsKeyDown('W')) moveDir += forward;
    if (input.IsKeyDown('S')) moveDir -= forward;
    if (input.IsKeyDown('D')) moveDir += right;
    if (input.IsKeyDown('A')) moveDir -= right;

    moveDir = XMVector3Normalize(moveDir);

    // 3. Apply Velocity
    m_velocity.x = XMVectorGetX(moveDir) * MOVE_SPEED;
    m_velocity.z = XMVectorGetZ(moveDir) * MOVE_SPEED;

    // Jumping
    if (m_onGround && input.IsKeyDown(VK_SPACE))
    {
        m_velocity.y = JUMP_FORCE;
        m_onGround = false;
    }

    // Gravity
    m_velocity.y += GRAVITY * deltaTime;

    // 4. Axis-Separated Movement & Collision (Prevents getting stuck)
    XMFLOAT3 startPos = GetPosition();

    // --- Move X ---
    SetPosition(startPos.x + m_velocity.x * deltaTime, startPos.y, startPos.z);
    for (const auto& obj : worldObjects) {
        if (obj.get() == this) continue;
        if (PhysicsSystem::AABBIntersects(GetWorldBoundingBox(), obj->GetWorldBoundingBox())) {
            SetPosition(startPos.x, startPos.y, startPos.z); // Revert X
            break;
        }
    }

    // --- Move Z ---
    startPos = GetPosition(); // Update start pos
    SetPosition(startPos.x, startPos.y, startPos.z + m_velocity.z * deltaTime);
    for (const auto& obj : worldObjects) {
        if (obj.get() == this) continue;
        if (PhysicsSystem::AABBIntersects(GetWorldBoundingBox(), obj->GetWorldBoundingBox())) {
            SetPosition(startPos.x, startPos.y, startPos.z); // Revert Z
            break;
        }
    }

    // --- Move Y ---
    startPos = GetPosition();
    SetPosition(startPos.x, startPos.y + m_velocity.y * deltaTime, startPos.z);
    m_onGround = false; // Assume in air until collision
    for (const auto& obj : worldObjects) {
        if (obj.get() == this) continue;
        if (PhysicsSystem::AABBIntersects(GetWorldBoundingBox(), obj->GetWorldBoundingBox())) {
            // If falling down and hit something, we are on ground
            if (m_velocity.y < 0) m_onGround = true;

            SetPosition(startPos.x, startPos.y, startPos.z); // Revert Y
            m_velocity.y = 0; // Stop gravity accumulation
            break;
        }
    }

    // 5. Update Camera Position to match Player Head
    XMFLOAT3 finalPos = GetPosition();
    m_camera->SetPosition(finalPos.x, finalPos.y + 0.8f, finalPos.z); // +0.8f for eye height
}