#pragma once

#include "Events/Event.h"
#include "ECS/Entity.h"
#include <typeindex>

// ==================================================================================
// ECS Component Events
// ----------------------------------------------------------------------------------
// Events fired when components are added, removed, or modified.
// Systems can subscribe to these events to react to component changes.
// ==================================================================================

struct ComponentAddedEvent : public Event {
    ECS::Entity entity;
    std::type_index componentType;
    
    ComponentAddedEvent(ECS::Entity e, std::type_index type)
        : entity(e), componentType(type) {}
    
    EVENT_CLASS_TYPE(ComponentAdded)
    EVENT_CLASS_CATEGORY(EventCategoryECS)
};

struct ComponentRemovedEvent : public Event {
    ECS::Entity entity;
    std::type_index componentType;
    
    ComponentRemovedEvent(ECS::Entity e, std::type_index type)
        : entity(e), componentType(type) {}
    
    EVENT_CLASS_TYPE(ComponentRemoved)
    EVENT_CLASS_CATEGORY(EventCategoryECS)
};

struct EntityDestroyedEvent : public Event {
    ECS::Entity entity;
    
    explicit EntityDestroyedEvent(ECS::Entity e)
        : entity(e) {}
    
    EVENT_CLASS_TYPE(EntityDestroyed)
    EVENT_CLASS_CATEGORY(EventCategoryECS)
};
