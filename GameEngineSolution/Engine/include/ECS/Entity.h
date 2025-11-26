#pragma once

#include "EntityHandle.h"
#include <cstdint>
#include <vector>
#include <stdexcept>

namespace ECS {

// Entity is now an EntityHandle with versioning
using Entity = EntityHandle;
constexpr Entity NULL_ENTITY = NULL_ENTITY_HANDLE;
constexpr uint32_t MAX_ENTITIES = 5000;

// ==================================================================================
// EntityIDGenerator
// ----------------------------------------------------------------------------------
// Manages entity ID generation, recycling, and versioning.
// When an entity is destroyed, its ID goes into a free list but the version increments.
// This prevents stale entity handles from being used after the entity is destroyed.
// ==================================================================================
class EntityIDGenerator {
public:
    EntityIDGenerator() : m_nextID(1) {
        m_versions.resize(MAX_ENTITIES, 0);
    }
    
    // Create a new entity handle
    Entity Create() {
        uint32_t id;
        
        if (!m_freeList.empty()) {
            // Reuse recycled ID
            id = m_freeList.back();
            m_freeList.pop_back();
        } else {
            // Generate new ID
            id = m_nextID++;
        }
        
        if (id >= MAX_ENTITIES) {
            throw std::runtime_error("Maximum entity count exceeded");
        }
        
        // Return handle with current version for this ID
        return EntityHandle{ id, m_versions[id] };
    }
    
    // Mark entity handle as destroyed
    void Destroy(Entity entity) {
        if (!entity.IsValid()) return;
        
        uint32_t id = entity.id;
        if (id >= MAX_ENTITIES) return;
        
        // Verify this is the current version (prevent double-free)
        if (m_versions[id] != entity.version) {
            return; // Stale handle, already destroyed
        }
        
        // Increment version for this ID (invalidates old handles)
        m_versions[id]++;
        
        // Add to free list for reuse
        m_freeList.push_back(id);
    }
    
    // Check if an entity handle is still valid
    bool IsValid(Entity entity) const {
        if (!entity.IsValid()) return false;
        if (entity.id >= MAX_ENTITIES) return false;
        return m_versions[entity.id] == entity.version;
    }
    
    // Get total number of IDs created (including recycled)
    uint32_t GetTotalCreated() const {
        return m_nextID - 1;
    }

    // Get number of currently active entities
    uint32_t GetActiveCount() const {
        return (m_nextID - 1) - static_cast<uint32_t>(m_freeList.size());
    }
    
private:
    uint32_t m_nextID;                  // Next ID to assign
    std::vector<uint32_t> m_freeList;   // Recycled IDs
    std::vector<uint32_t> m_versions;   // Current version for each ID
};

} // namespace ECS

