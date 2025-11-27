#include "../../include/ECS/ComponentManager.h"
#include "../../include/Events/ECSEvents.h"
#include "../../include/Events/EventBus.h"

#include "../../include/Utils/Logger.h"

namespace ECS {

void ComponentManager::FireComponentAddedEvent(Entity entity, std::type_index componentType) {
    if (!m_eventBus) {
        LOG_WARNING("ComponentManager: EventBus is null!");
        return;
    }
    
    LOG_INFO("ComponentManager: Firing ComponentAdded for Entity " + std::to_string(entity.id));
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
