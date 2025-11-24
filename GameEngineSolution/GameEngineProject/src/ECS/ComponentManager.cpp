#include "../../include/ECS/ComponentManager.h"
#include <algorithm>

namespace ECS {

// ========================================
// Entity Management
// ========================================

Entity ComponentManager::CreateEntity() {
    Entity entity = m_idGenerator.Create();
    m_entities.push_back(entity);
    return entity;
}

void ComponentManager::DestroyEntity(Entity entity) {
    // Remove all components for this entity
    RemoveTransform(entity);
    RemovePhysics(entity);
    RemoveRender(entity);
    RemoveCollider(entity);
    
    // Remove from entity list
    auto it = std::find(m_entities.begin(), m_entities.end(), entity);
    if (it != m_entities.end()) {
        m_entities.erase(it);
    }
    
    // Recycle ID
    m_idGenerator.Destroy(entity);
}

// ========================================
// Transform Component
// ========================================

void ComponentManager::AddTransform(Entity entity, const TransformComponent& component) {
    // Check if entity already has this component
    if (m_entityToTransform.find(entity) != m_entityToTransform.end()) {
        // Update existing component
        size_t index = m_entityToTransform[entity];
        m_transforms[index] = component;
        return;
    }
    
    // Add new component
    size_t index = m_transforms.size();
    m_transforms.push_back(component);
    m_entityToTransform[entity] = index;
    m_transformToEntity[index] = entity;
}

void ComponentManager::RemoveTransform(Entity entity) {
    auto it = m_entityToTransform.find(entity);
    if (it == m_entityToTransform.end()) return;  // Component doesn't exist
    
    size_t index = it->second;
    size_t lastIndex = m_transforms.size() - 1;
    
    if (index != lastIndex) {
        // Swap with last element
        m_transforms[index] = m_transforms[lastIndex];
        
        // Update mappings for swapped element
        Entity swappedEntity = m_transformToEntity[lastIndex];
        m_entityToTransform[swappedEntity] = index;
        m_transformToEntity[index] = swappedEntity;
    }
    
    // Remove last element
    m_transforms.pop_back();
    m_entityToTransform.erase(entity);
    m_transformToEntity.erase(lastIndex);
}

TransformComponent* ComponentManager::GetTransform(Entity entity) {
    auto it = m_entityToTransform.find(entity);
    if (it == m_entityToTransform.end()) return nullptr;
    return &m_transforms[it->second];
}

bool ComponentManager::HasTransform(Entity entity) const {
    return m_entityToTransform.find(entity) != m_entityToTransform.end();
}

// ========================================
// Physics Component
// ========================================

void ComponentManager::AddPhysics(Entity entity, const PhysicsComponent& component) {
    if (m_entityToPhysics.find(entity) != m_entityToPhysics.end()) {
        size_t index = m_entityToPhysics[entity];
        m_physics[index] = component;
        return;
    }
    
    size_t index = m_physics.size();
    m_physics.push_back(component);
    m_entityToPhysics[entity] = index;
    m_physicsToEntity[index] = entity;
}

void ComponentManager::RemovePhysics(Entity entity) {
    auto it = m_entityToPhysics.find(entity);
    if (it == m_entityToPhysics.end()) return;
    
    size_t index = it->second;
    size_t lastIndex = m_physics.size() - 1;
    
    if (index != lastIndex) {
        m_physics[index] = m_physics[lastIndex];
        Entity swappedEntity = m_physicsToEntity[lastIndex];
        m_entityToPhysics[swappedEntity] = index;
        m_physicsToEntity[index] = swappedEntity;
    }
    
    m_physics.pop_back();
    m_entityToPhysics.erase(entity);
    m_physicsToEntity.erase(lastIndex);
}

PhysicsComponent* ComponentManager::GetPhysics(Entity entity) {
    auto it = m_entityToPhysics.find(entity);
    if (it == m_entityToPhysics.end()) return nullptr;
    return &m_physics[it->second];
}

bool ComponentManager::HasPhysics(Entity entity) const {
    return m_entityToPhysics.find(entity) != m_entityToPhysics.end();
}

// ========================================
// Render Component
// ========================================

void ComponentManager::AddRender(Entity entity, const RenderComponent& component) {
    if (m_entityToRender.find(entity) != m_entityToRender.end()) {
        size_t index = m_entityToRender[entity];
        m_renders[index] = component;
        return;
    }
    
    size_t index = m_renders.size();
    m_renders.push_back(component);
    m_entityToRender[entity] = index;
    m_renderToEntity[index] = entity;
}

void ComponentManager::RemoveRender(Entity entity) {
    auto it = m_entityToRender.find(entity);
    if (it == m_entityToRender.end()) return;
    
    size_t index = it->second;
    size_t lastIndex = m_renders.size() - 1;
    
    if (index != lastIndex) {
        m_renders[index] = m_renders[lastIndex];
        Entity swappedEntity = m_renderToEntity[lastIndex];
        m_entityToRender[swappedEntity] = index;
        m_renderToEntity[index] = swappedEntity;
    }
    
    m_renders.pop_back();
    m_entityToRender.erase(entity);
    m_renderToEntity.erase(lastIndex);
}

RenderComponent* ComponentManager::GetRender(Entity entity) {
    auto it = m_entityToRender.find(entity);
    if (it == m_entityToRender.end()) return nullptr;
    return &m_renders[it->second];
}

bool ComponentManager::HasRender(Entity entity) const {
    return m_entityToRender.find(entity) != m_entityToRender.end();
}

// ========================================
// Collider Component
// ========================================

void ComponentManager::AddCollider(Entity entity, const ColliderComponent& component) {
    if (m_entityToCollider.find(entity) != m_entityToCollider.end()) {
        size_t index = m_entityToCollider[entity];
        m_colliders[index] = component;
        return;
    }
    
    size_t index = m_colliders.size();
    m_colliders.push_back(component);
    m_entityToCollider[entity] = index;
    m_colliderToEntity[index] = entity;
}

void ComponentManager::RemoveCollider(Entity entity) {
    auto it = m_entityToCollider.find(entity);
    if (it == m_entityToCollider.end()) return;
    
    size_t index = it->second;
    size_t lastIndex = m_colliders.size() - 1;
    
    if (index != lastIndex) {
        m_colliders[index] = m_colliders[lastIndex];
        Entity swappedEntity = m_colliderToEntity[lastIndex];
        m_entityToCollider[swappedEntity] = index;
        m_colliderToEntity[index] = swappedEntity;
    }
    
    m_colliders.pop_back();
    m_entityToCollider.erase(entity);
    m_colliderToEntity.erase(lastIndex);
}

ColliderComponent* ComponentManager::GetCollider(Entity entity) {
    auto it = m_entityToCollider.find(entity);
    if (it == m_entityToCollider.end()) return nullptr;
    return &m_colliders[it->second];
}

bool ComponentManager::HasCollider(Entity entity) const {
    return m_entityToCollider.find(entity) != m_entityToCollider.end();
}

// ========================================
// Queries
// ========================================

std::vector<Entity> ComponentManager::GetEntitiesWithTransform() const {
    std::vector<Entity> result;
    result.reserve(m_transforms.size());
    
    for (const auto& [index, entity] : m_transformToEntity) {
        result.push_back(entity);
    }
    
    return result;
}

std::vector<Entity> ComponentManager::GetEntitiesWithPhysics() const {
    std::vector<Entity> result;
    result.reserve(m_physics.size());
    
    for (const auto& [index, entity] : m_physicsToEntity) {
        result.push_back(entity);
    }
    
    return result;
}

std::vector<Entity> ComponentManager::GetEntitiesWithRender() const {
    std::vector<Entity> result;
    result.reserve(m_renders.size());
    
    for (const auto& [index, entity] : m_renderToEntity) {
        result.push_back(entity);
    }
    
    return result;
}

std::vector<Entity> ComponentManager::GetEntitiesWithCollider() const {
    std::vector<Entity> result;
    result.reserve(m_colliders.size());
    
    for (const auto& [index, entity] : m_colliderToEntity) {
        result.push_back(entity);
    }
    
    return result;
}

std::vector<Entity> ComponentManager::GetEntitiesWithPhysicsAndTransform() const {
    std::vector<Entity> result;
    
    // Iterate through physics entities (usually smaller set)
    for (const auto& [index, entity] : m_physicsToEntity) {
        if (HasTransform(entity)) {
            result.push_back(entity);
        }
    }
    
    return result;
}

std::vector<Entity> ComponentManager::GetEntitiesWithRenderAndTransform() const {
    std::vector<Entity> result;
    
    // Iterate through render entities
    for (const auto& [index, entity] : m_renderToEntity) {
        if (HasTransform(entity)) {
            result.push_back(entity);
        }
    }
    
    return result;
}

} // namespace ECS
