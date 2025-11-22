#pragma once
#include "GameObject.h"
#include "Input.h"     // Assuming you have an Input header
#include "Camera.h"    // Assuming you have a Camera header

class Player : public GameObject
{
public:
    Player(Mesh* mesh, std::shared_ptr<Material> material, Camera* camera);

    void Update(float deltaTime, Input& input, const std::vector<std::unique_ptr<GameObject>>& worldObjects);

private:
    Camera* m_camera;
    DirectX::XMFLOAT3 m_velocity;
    bool m_onGround;

    // Constants
    const float MOVE_SPEED = 5.0f;
    const float JUMP_FORCE = 5.0f;
    const float GRAVITY = -15.0f; // Higher gravity feels better in games
};