#include "../../include/UI/DebugUIRenderer.h"
#include "../../include/UI/UIRenderer.h"
#include <string>

DebugUIRenderer::DebugUIRenderer() : m_enabled(true)
{
}

void DebugUIRenderer::Render(
    UIRenderer* uiRenderer,
    const SimpleFont& font,
    int fps,
    bool bloomEnabled,
    bool debugCollisionEnabled,
    ECS::ComponentManager& componentManager
)
{
    if (!m_enabled || !uiRenderer) return;

    // Color definitions
    float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float yellow[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
    float cyan[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
    float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float orange[4] = { 1.0f, 0.5f, 0.0f, 1.0f };

    float yPos = 10.0f;
    const float lineHeight = 30.0f;

    // Draw FPS
    std::string fpsString = "FPS: " + std::to_string(fps);
    uiRenderer->DrawString(font, fpsString, 10.0f, yPos, 24.0f, green);
    yPos += lineHeight;

    // Draw Bloom status
    std::string bloomStatus = bloomEnabled ? "[B] Bloom: ON" : "[B] Bloom: OFF";
    uiRenderer->DrawString(font, bloomStatus, 10.0f, yPos, 24.0f, yellow);
    yPos += lineHeight;

    // Draw Debug Collision status
    std::string debugStatus = debugCollisionEnabled ? "[H] Debug: ON" : "[H] Debug: OFF";
    uiRenderer->DrawString(font, debugStatus, 10.0f, yPos, 24.0f, cyan);
    yPos += lineHeight;

    // Draw Debug UI toggle hint
    std::string debugUIHint = "[F1] Debug UI: ON";
    uiRenderer->DrawString(font, debugUIHint, 10.0f, yPos, 24.0f, orange);
    yPos += lineHeight;

    // Draw entity count
    size_t entityCount = componentManager.GetEntityCount();
    std::string entityInfo = "ECS Entities: " + std::to_string(entityCount);
    uiRenderer->DrawString(font, entityInfo, 10.0f, yPos, 20.0f, white);
    yPos += lineHeight;

    // Show player information if exists
    std::vector<ECS::Entity> players = componentManager.GetEntitiesWithPlayerController();
    if (!players.empty()) {
        ECS::TransformComponent* playerTrans = componentManager.GetTransform(players[0]);
        ECS::PhysicsComponent* playerPhys = componentManager.GetPhysics(players[0]);

        if (playerTrans) {
            char playerPosBuffer[128];
            snprintf(playerPosBuffer, sizeof(playerPosBuffer), "Player Pos: (%.2f, %.2f, %.2f)",
                playerTrans->position.x, playerTrans->position.y, playerTrans->position.z);
            uiRenderer->DrawString(font, playerPosBuffer, 10.0f, yPos, 20.0f, green);
            yPos += lineHeight;
        }

        if (playerPhys) {
            char playerVelBuffer[128];
            snprintf(playerVelBuffer, sizeof(playerVelBuffer), "Velocity: (%.2f, %.2f, %.2f) Grounded: %s",
                playerPhys->velocity.x, playerPhys->velocity.y, playerPhys->velocity.z,
                playerPhys->isGrounded ? "YES" : "NO");
            uiRenderer->DrawString(font, playerVelBuffer, 10.0f, yPos, 18.0f, yellow);
            yPos += lineHeight;
        }
    }
}
