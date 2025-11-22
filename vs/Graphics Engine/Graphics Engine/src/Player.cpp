#include "include/Player.h"
#include "include/Scene.h" 
#include "include/PhysicsSystem.h"
#include "include/Bullet.h"

using namespace DirectX;

Player::Player(Mesh* mesh, std::shared_ptr<Material> material, Camera* camera)
    : GameObject(mesh, material), m_camera(camera), m_velocity(0.0f, 0.0f, 0.0f), m_onGround(false)
{
    // Set a smaller bounding box for the player (human size)
    // Center at 0.9 means feet at Y=0 (ground), top of head at Y=1.8 (6ft tall human)
    AABB playerBox;
    playerBox.center = { 0.0f, 0.9f, 0.0f };  // Fixed: was 1.0, causing gap at bottom
    playerBox.extents = { 0.4f, 0.9f, 0.4f };
    SetBoundingBox(playerBox);
}

void Player::Update(float deltaTime, Input& input, const std::vector<std::unique_ptr<GameObject>>& worldObjects)
{
    // --- Rotation (Mouse Look) ---
    float mouseSens = 0.002f;
    int dx = input.GetMouseDeltaX();
    int dy = input.GetMouseDeltaY();

    // Rotate Player (Y-axis)
    float rotY = GetRotation().y + dx * mouseSens;
    SetRotation(GetRotation().x, rotY, GetRotation().z);

    // Rotate Camera (X-axis, clamped)
    m_camera->AdjustRotation(dy * mouseSens, dx * mouseSens, 0.0f);

    // Sync Camera Position to Player (First Person)
    DirectX::XMFLOAT3 pos = GetPosition();
    m_camera->SetPosition(pos.x, pos.y + 0.7f, pos.z); // Eye level (0.7 = 1.6 eye height - 0.9 center offset)

    // --- Movement (WASD) ---
    float yaw = GetRotation().y;
    DirectX::XMFLOAT3 forward = { sinf(yaw), 0.0f, cosf(yaw) };
    DirectX::XMFLOAT3 right = { cosf(yaw), 0.0f, -sinf(yaw) };

    DirectX::XMFLOAT3 moveDir = { 0.0f, 0.0f, 0.0f };
    if (input.IsKeyDown('W')) { moveDir.x += forward.x; moveDir.z += forward.z; }
    if (input.IsKeyDown('S')) { moveDir.x -= forward.x; moveDir.z -= forward.z; }
    if (input.IsKeyDown('A')) { moveDir.x -= right.x; moveDir.z -= right.z; }
    if (input.IsKeyDown('D')) { moveDir.x += right.x; moveDir.z += right.z; }

    // Normalize
    float length = sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
    if (length > 0.0f) {
        moveDir.x /= length;
        moveDir.z /= length;
    }

    m_velocity.x = moveDir.x * MOVE_SPEED;
    m_velocity.z = moveDir.z * MOVE_SPEED;

    // --- Jumping ---
    if (m_onGround && input.IsKeyDown(VK_SPACE)) {
        m_velocity.y = JUMP_FORCE;
        m_onGround = false;
    }

    // --- Gravity ---
    m_velocity.y += GRAVITY * deltaTime;

    DirectX::XMFLOAT3 startPos = GetPosition();

    // --- Move X/Z ---
    SetPosition(startPos.x + m_velocity.x * deltaTime, startPos.y, startPos.z + m_velocity.z * deltaTime);
    
    for (const auto& obj : worldObjects) {
        if (obj.get() == this || obj.get() == m_gunPtr || dynamic_cast<Bullet*>(obj.get())) continue;
        if (PhysicsSystem::AABBIntersects(GetWorldBoundingBox(), obj->GetWorldBoundingBox())) {
            SetPosition(startPos.x, startPos.y, startPos.z);
            break;
        }
    }

    // --- Move Y (PREDICTIVE) ---
    startPos = GetPosition();
    if (startPos.y < -20.0f) {
        SetPosition(0.0f, 5.0f, 0.0f);
        m_velocity = { 0.0f, 0.0f, 0.0f };
        return;
    }

    float intendedY = startPos.y + m_velocity.y * deltaTime;
    const float skinWidth = 0.005f;
    bool collisionDetected = false;

    AABB playerBox = GetWorldBoundingBox();
    AABB intendedBox = playerBox;
    float localOffset = playerBox.center.y - startPos.y;
    intendedBox.center.y = intendedY + localOffset;

    for (const auto& obj : worldObjects) {
        if (obj.get() == this || obj.get() == m_gunPtr || dynamic_cast<Bullet*>(obj.get())) continue;
        
        AABB objBox = obj->GetWorldBoundingBox();
        if (PhysicsSystem::AABBIntersects(intendedBox, objBox)) {
            collisionDetected = true;

            if (m_velocity.y < 0) {
                // LANDING: Position so that AABB bottom = objectTop
                // AABB bottom = (transform.y + localOffset) - extents.y
                // We want: (transform.y + localOffset) - extents.y = objectTop
                // So: transform.y = objectTop - localOffset + extents.y
                float objectTop = objBox.center.y + objBox.extents.y;
                float resolvedY = objectTop - localOffset + playerBox.extents.y;
                SetPosition(startPos.x, resolvedY, startPos.z);
                m_velocity.y = 0;
            }
            else if (m_velocity.y > 0) {
                // HEAD BUMP
                float objectBottom = objBox.center.y - objBox.extents.y;
                float resolvedY = objectBottom - localOffset - playerBox.extents.y;
                SetPosition(startPos.x, resolvedY, startPos.z);
                m_velocity.y = 0;
            }
            break;
        }
    }

    if (!collisionDetected) {
        SetPosition(startPos.x, intendedY, startPos.z);
    }

    // --- GROUND CHECK (Foot Probe) ---
    m_onGround = false;
    AABB footProbe = GetWorldBoundingBox();
    footProbe.center.y -= (skinWidth * 2.0f);

    for (const auto& obj : worldObjects) {
        if (obj.get() == this || obj.get() == m_gunPtr || dynamic_cast<Bullet*>(obj.get())) continue;
        if (PhysicsSystem::AABBIntersects(footProbe, obj->GetWorldBoundingBox())) {
            m_onGround = true;
            break;
        }
    }

    // --- Update Gun ---
    if (m_gunPtr) {
        DirectX::XMFLOAT3 camPos = m_camera->GetPositionFloat3();
        DirectX::XMVECTOR camForwardVec = m_camera->GetForward();
        DirectX::XMVECTOR camRightVec = m_camera->GetRight();
        DirectX::XMVECTOR camUpVec = m_camera->GetUp();

        DirectX::XMFLOAT3 camForward, camRight, camUp;
        XMStoreFloat3(&camForward, camForwardVec);
        XMStoreFloat3(&camRight, camRightVec);
        XMStoreFloat3(&camUp, camUpVec);

        DirectX::XMFLOAT3 gunPos;
        gunPos.x = camPos.x + camRight.x * 0.3f - camUp.x * 0.2f + camForward.x * 0.5f;
        gunPos.y = camPos.y + camRight.y * 0.3f - camUp.y * 0.2f + camForward.y * 0.5f;
        gunPos.z = camPos.z + camRight.z * 0.3f - camUp.z * 0.2f + camForward.z * 0.5f;

        m_gunPtr->SetPosition(gunPos.x, gunPos.y, gunPos.z);
        
        DirectX::XMVECTOR camRotVec = m_camera->GetRotation();
        DirectX::XMFLOAT3 camRot;
        XMStoreFloat3(&camRot, camRotVec);
        m_gunPtr->SetRotation(camRot.x, camRot.y, camRot.z);

        m_gunPtr->Update(deltaTime);
    }
}

void Player::Shoot(Scene* sceneInstance)
{
    if (m_gunPtr) {
        DirectX::XMVECTOR shootDirVec = m_camera->GetForward();
        DirectX::XMFLOAT3 shootDir;
        XMStoreFloat3(&shootDir, shootDirVec);

        DirectX::XMFLOAT3 shootPos = m_gunPtr->GetPosition();
        shootPos.x += shootDir.x * 0.5f;
        shootPos.y += shootDir.y * 0.5f;
        shootPos.z += shootDir.z * 0.5f;

        m_gunPtr->Shoot(sceneInstance, shootPos, shootDir);
    }
}