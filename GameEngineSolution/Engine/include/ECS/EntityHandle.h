#pragma once

#include <cstdint>
#include <functional>

namespace ECS {

// ==================================================================================
// EntityHandle
// ----------------------------------------------------------------------------------
// Type-safe entity handle with versioning to prevent use-after-free bugs.
// When an entity is destroyed, its ID is recycled but the version increments.
// Old handles to destroyed entities will have mismatched versions and be invalid.
// ==================================================================================
struct EntityHandle {
    uint32_t id = 0;
    uint32_t version = 0;
    
    bool IsValid() const {
        return id != 0;
    }
    
    bool operator==(const EntityHandle& other) const {
        return id == other.id && version == other.version;
    }
    
    bool operator!=(const EntityHandle& other) const {
        return !(*this == other);
    }
    
    bool operator<(const EntityHandle& other) const {
        if (id != other.id) return id < other.id;
        return version < other.version;
    }
};

constexpr EntityHandle NULL_ENTITY_HANDLE = { 0, 0 };

} // namespace ECS

// Hash specialization for EntityHandle
namespace std {
    template<>
    struct hash<ECS::EntityHandle> {
        size_t operator()(const ECS::EntityHandle& handle) const noexcept {
            // Combine id and version into single hash
            size_t h1 = std::hash<uint32_t>{}(handle.id);
            size_t h2 = std::hash<uint32_t>{}(handle.version);
            return h1 ^ (h2 << 1);
        }
    };
}
