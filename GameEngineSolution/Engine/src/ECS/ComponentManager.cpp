#include "../../include/ECS/ComponentManager.h"
#include "../../include/Events/ECSEvents.h"
#include "../../include/Events/EventBus.h"

namespace ECS {

void ComponentManager::FireComponentAddedEvent(Entity entity, std::type_index componentType) {
    if (!m_eventBus) return;
    
    ComponentAddedEvent event(entity, componentType);
    m_eventBus->Publish(event);
}

void ComponentManager::FireComponentRemovedEvent(Entity entity, std::type_index componentType) {
    if (!m_eventBus) return;
    
    ComponentRemovedEvent event(entity, componentType);
    m_eventBus->Publish(event);
}

void ComponentManager::FireEntityDestroyedEvent(Entity entity) {
    if (!m_eventBus) return;
    
    EntityDestroyedEvent event(entity);
    m_eventBus->Publish(event);
}

} // namespace ECS
