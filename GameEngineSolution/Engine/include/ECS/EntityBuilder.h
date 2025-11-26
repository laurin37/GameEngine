#pragma once

#include "ComponentManager.h"

namespace ECS {

// ==================================================================================
// EntityBuilder
// ----------------------------------------------------------------------------------
// Fluent API for creating entities with multiple components in a clean, readable way.
// 
// Example Usage:
//     auto player = EntityBuilder(componentManager)
//         .With(TransformComponent{ {0, 0, 0} })
//         .With(PhysicsComponent{})
//         .With(PlayerControllerComponent{})
//         .Build();
// ==================================================================================
class EntityBuilder {
public:
    explicit EntityBuilder(ComponentManager& componentManager)
        : m_componentManager(componentManager)
        , m_entity(componentManager.CreateEntity()) {
    }
    
    // Add a component to the entity being built
    template<typename T>
    EntityBuilder& With(T component) {
        m_componentManager.AddComponent(m_entity, component);
        return *this;
    }
    
    // Conditionally add a component
    template<typename T>
    EntityBuilder& WithIf(bool condition, T component) {
        if (condition) {
            m_componentManager.AddComponent(m_entity, component);
        }
        return *this;
    }
    
    // Finish building and return the entity
    Entity Build() {
        return m_entity;
    }
    
    // Alternative: Build and execute a setup function
    template<typename Func>
    Entity BuildWith(Func&& setupFunc) {
        setupFunc(m_entity, m_componentManager);
        return m_entity;
    }
    
private:
    ComponentManager& m_componentManager;
    Entity m_entity;
};

} // namespace ECS
