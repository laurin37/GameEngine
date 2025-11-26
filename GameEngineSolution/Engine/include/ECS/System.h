#pragma once

#include "ComponentManager.h"
#include "SystemPhase.h"
#include <typeindex>

// Forward declarations
class EventBus;

namespace ECS {

// ==================================================================================
// System Base Class
// ----------------------------------------------------------------------------------
// All ECS systems inherit from this class.
// Systems process entities with specific component combinations each frame.
// 
// New Features:
// - System phases for execution order control
// - Parallelization support flag
// - Component event callbacks
// - Cached component arrays for performance
// ==================================================================================
class System {
public:
    explicit System(ComponentManager& componentManager) 
        : m_componentManager(componentManager) {}
    
    virtual ~System() = default;

    // Lifecycle methods
    virtual void Init() {}
    virtual void Update(float deltaTime) {}
    virtual void Shutdown() {}
    
    // System scheduling
    virtual SystemPhase GetPhase() const { return SystemPhase::Update; }
    virtual bool CanParallelize() const { return false; }  // Override to true if thread-safe
    
    // Component event callbacks (optional)
    virtual void OnComponentAdded(Entity entity, std::type_index componentType) {}
    virtual void OnComponentRemoved(Entity entity, std::type_index componentType) {}
    virtual void OnEntityDestroyed(Entity entity) {}
    
    // Event bus integration
    void SetEventBus(EventBus* eventBus) {
        m_eventBus = eventBus;
    }

protected:
    ComponentManager& m_componentManager;
    EventBus* m_eventBus = nullptr;
};

} // namespace ECS

