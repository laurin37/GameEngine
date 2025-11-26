#pragma once

namespace ECS {

// ==================================================================================
// SystemPhase Enumeration
// ----------------------------------------------------------------------------------
// Defines the execution order of systems within the game loop.
// Systems are grouped by phase and executed in this order each frame.
// ==================================================================================
enum class SystemPhase : int {
    PreUpdate = 0,   // Input handling, event processing
    Update = 1,      // Game logic, AI, movement decisions
    PostUpdate = 2,  // Physics integration, collision resolution
    PreRender = 3,   // Animation, culling, render cache updates
    Render = 4       // Actual rendering (typically not ECS systems)
};

} // namespace ECS
