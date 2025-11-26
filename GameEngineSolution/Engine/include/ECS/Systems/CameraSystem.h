#pragma once

#include "../ComponentManager.h"
#include "../System.h"
#include "../SystemPhase.h"
#include <DirectXMath.h>
#include <memory>

namespace ECS {

// ==================================================================================
// CameraSystem
// ----------------------------------------------------------------------------------
// Updates camera view and projection matrices
class CameraSystem : public System {
public:
    explicit CameraSystem(ComponentManager& cm) : System(cm) {}

    void Init() override;
    void Update(float deltaTime) override;
    
    // System phase
    SystemPhase GetPhase() const override { return SystemPhase::PreRender; }
    bool CanParallelize() const override { return true; } // Read-only operations
    
    // Get active camera's matrices  (returns false if no active camera)
    bool GetActiveCamera(DirectX::XMMATRIX& viewOut, DirectX::XMMATRIX& projOut);

    // Get the active camera entity ID
    Entity GetActiveCameraEntity();
    
private:
    // Cached component arrays
    std::shared_ptr<ComponentArray<CameraComponent>> m_cameraArray;
    std::shared_ptr<ComponentArray<TransformComponent>> m_transformArray;
};

}
