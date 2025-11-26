#pragma once

#include "Entity.h"
#include "Components.h"
#include <unordered_map>
#include <vector>
#include <optional>
#include <typeindex>
#include <memory>
#include <stdexcept>
#include <cassert>
#include <bitset>
#include <shared_mutex>
#include <algorithm>

// Forward declare EventBus
class EventBus;

namespace ECS {

// ========================================
// Component Signature
// ========================================
constexpr size_t MAX_COMPONENTS = 64;
using Signature = std::bitset<MAX_COMPONENTS>;

// ========================================
// IComponentArray
// Interface for generic component arrays
// ========================================
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(Entity entity) = 0;
    virtual size_t GetSize() const = 0;
};

// ========================================
// ComponentArray<T>
// Generic sparse set storage for components
// Thread-safe with read/write locks
// ========================================
template<typename T>
class ComponentArray : public IComponentArray {
public:
    ComponentArray() {
        // Initialize sparse array with invalid index
        m_entityToIndex.resize(MAX_ENTITIES, INVALID_INDEX);
        
        // Pre-allocate for typical scene size
        m_componentArray.reserve(1024);
        m_indexToEntity.reserve(1024);
    }

    void InsertData(Entity entity, T component) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        
        uint32_t id = entity.id;
        if (id >= MAX_ENTITIES) {
            throw std::runtime_error("Entity ID out of range.");
        }

        if (m_entityToIndex[id] != INVALID_INDEX) {
            // Component already exists, just update it
            m_componentArray[m_entityToIndex[id]] = component;
            return;
        }

        // Add new component
        size_t newIndex = m_size;
        m_entityToIndex[id] = static_cast<uint32_t>(newIndex);
        m_indexToEntity.push_back(entity);
        m_componentArray.push_back(component);
        m_size++;
    }

    void RemoveData(Entity entity) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        
        uint32_t id = entity.id;
        if (id >= MAX_ENTITIES || m_entityToIndex[id] == INVALID_INDEX) {
            return; // Entity doesn't have this component
        }

        // Copy last element into deleted element's place to maintain density
        size_t indexOfRemovedEntity = m_entityToIndex[id];
        size_t indexOfLastElement = m_size - 1;
        
        m_componentArray[indexOfRemovedEntity] = m_componentArray[indexOfLastElement];

        // Update map to point to moved spot
        Entity entityOfLastElement = m_indexToEntity[indexOfLastElement];
        m_entityToIndex[entityOfLastElement.id] = static_cast<uint32_t>(indexOfRemovedEntity);
        m_indexToEntity[indexOfRemovedEntity] = entityOfLastElement;

        m_entityToIndex[id] = INVALID_INDEX;
        m_indexToEntity.pop_back();
        m_componentArray.pop_back();

        m_size--;
    }

    T& GetData(Entity entity) {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        
        uint32_t id = entity.id;
        if (id >= MAX_ENTITIES || m_entityToIndex[id] == INVALID_INDEX) {
            throw std::runtime_error("Retrieving non-existent component.");
        }
        return m_componentArray[m_entityToIndex[id]];
    }
    
    const T& GetData(Entity entity) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        
        uint32_t id = entity.id;
        if (id >= MAX_ENTITIES || m_entityToIndex[id] == INVALID_INDEX) {
            throw std::runtime_error("Retrieving non-existent component.");
        }
        return m_componentArray[m_entityToIndex[id]];
    }

    bool HasData(Entity entity) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        
        uint32_t id = entity.id;
        return id < MAX_ENTITIES && m_entityToIndex[id] != INVALID_INDEX;
    }

    void EntityDestroyed(Entity entity) override {
        uint32_t id = entity.id;
        if (id < MAX_ENTITIES && m_entityToIndex[id] != INVALID_INDEX) {
            RemoveData(entity);
        }
    }

    // Direct access for systems (read-only, thread-safe)
    std::vector<T> GetComponentArrayCopy() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_componentArray;
    }
    
    // Get component array for iteration (caller must hold lock)
    const std::vector<T>& GetComponentArray() const {
        return m_componentArray;
    }
    
    std::vector<T>& GetComponentArray() {
        return m_componentArray;
    }
    
    // Helper to get entity for a specific index
    Entity GetEntityAtIndex(size_t index) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        if (index >= m_indexToEntity.size()) {
            throw std::runtime_error("Index out of range");
        }
        return m_indexToEntity[index];
    }

    size_t GetSize() const override {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_size;
    }
    
    // Lock acquisition for batch operations
    std::shared_mutex& GetMutex() {
        return m_mutex;
    }

private:
    static constexpr uint32_t INVALID_INDEX = 0xFFFFFFFF;
    
    std::vector<T> m_componentArray;
    std::vector<uint32_t> m_entityToIndex;  // Sparse array: Entity ID -> Index
    std::vector<Entity> m_indexToEntity;    // Dense array: Index -> Entity
    size_t m_size = 0;
    mutable std::shared_mutex m_mutex;      // Read/write lock for thread safety
};

// ==================================================================================
// ComponentManager Class
// ----------------------------------------------------------------------------------
// The core of the ECS architecture.
// Responsible for:
// - Creating and destroying entities (with versioning)
// - Managing component arrays (Sparse Sets) for each component type
// - Tracking component signatures for efficient querying
// - Providing fast access to components for Systems
// - Thread-safe component operations
// - Firing component lifecycle events
// ==================================================================================
class ComponentManager {
public:
    ComponentManager() = default;
    ~ComponentManager() = default;
    
    // Set event bus for component events
    void SetEventBus(EventBus* eventBus) {
        m_eventBus = eventBus;
    }
    
    // ========================================
    // Component Type Registration
    // ========================================
    
    template<typename T>
    void RegisterComponent() {
        std::type_index typeIndex(typeid(T));
        
        if (m_componentTypeIDs.find(typeIndex) != m_componentTypeIDs.end()) {
            return; // Already registered
        }
        
        if (m_nextComponentTypeID >= MAX_COMPONENTS) {
            throw std::runtime_error("Maximum component types exceeded");
        }
        
        m_componentTypeIDs[typeIndex] = m_nextComponentTypeID++;
    }
    
    template<typename T>
    uint32_t GetComponentTypeID() {
        std::type_index typeIndex(typeid(T));
        auto it = m_componentTypeIDs.find(typeIndex);
        if (it == m_componentTypeIDs.end()) {
            // Auto-register if not registered
            RegisterComponent<T>();
            return m_componentTypeIDs[typeIndex];
        }
        return it->second;
    }
    
    // ========================================
    // Entity Management
    // ========================================
    Entity CreateEntity() {
        Entity entity = m_idGenerator.Create();
        if (entity.id >= MAX_ENTITIES) {
            throw std::runtime_error("Maximum entity count exceeded.");
        }
        
        // Initialize signature
        m_signatures[entity] = Signature();
        
        return entity;
    }

    void DestroyEntity(Entity entity) {
        if (!m_idGenerator.IsValid(entity)) {
            return; // Already destroyed or invalid
        }
        
        // Remove from signatures
        m_signatures.erase(entity);
        
        // Notify all component arrays
        for (auto const& pair : m_componentArrays) {
            auto& componentArray = pair.second;
            if (componentArray) {
                componentArray->EntityDestroyed(entity);
            }
        }
        
        // Fire entity destroyed event
        if (m_eventBus) {
            FireEntityDestroyedEvent(entity);
        }
        
        // Mark ID as destroyed
        m_idGenerator.Destroy(entity);
    }
    
    bool IsEntityValid(Entity entity) const {
        return m_idGenerator.IsValid(entity);
    }

    size_t GetEntityCount() const { return m_idGenerator.GetActiveCount(); }

    // ========================================
    // Component Management
    // ========================================
    
    template<typename T>
    void AddComponent(Entity entity, T component) {
        if (!m_idGenerator.IsValid(entity)) {
            throw std::runtime_error("Cannot add component to invalid entity");
        }
        
        GetComponentArray<T>()->InsertData(entity, component);
        
        // Update signature
        uint32_t componentTypeID = GetComponentTypeID<T>();
        m_signatures[entity].set(componentTypeID);
        
        // Fire component added event
        if (m_eventBus) {
            FireComponentAddedEvent(entity, typeid(T));
        }
    }

    template<typename T>
    void RemoveComponent(Entity entity) {
        if (!m_idGenerator.IsValid(entity)) {
            return;
        }
        
        GetComponentArray<T>()->RemoveData(entity);
        
        // Update signature
        uint32_t componentTypeID = GetComponentTypeID<T>();
        m_signatures[entity].reset(componentTypeID);
        
        // Fire component removed event
        if (m_eventBus) {
            FireComponentRemovedEvent(entity, typeid(T));
        }
    }

    template<typename T>
    T& GetComponent(Entity entity) {
        return GetComponentArray<T>()->GetData(entity);
    }
    
    template<typename T>
    const T& GetComponent(Entity entity) const {
        return GetComponentArray<T>()->GetData(entity);
    }
    
    template<typename T>
    T* GetComponentPtr(Entity entity) {
        auto array = GetComponentArray<T>();
        if (array->HasData(entity)) {
            return &array->GetData(entity);
        }
        return nullptr;
    }
    
    template<typename T>
    const T* GetComponentPtr(Entity entity) const {
        auto array = GetComponentArray<T>();
        if (array->HasData(entity)) {
            return &array->GetData(entity);
        }
        return nullptr;
    }

    template<typename T>
    bool HasComponent(Entity entity) const {
        if (!m_idGenerator.IsValid(entity)) {
            return false;
        }
        return GetComponentArray<T>()->HasData(entity);
    }
    
    // Get entity signature
    Signature GetSignature(Entity entity) const {
        auto it = m_signatures.find(entity);
        if (it != m_signatures.end()) {
            return it->second;
        }
        return Signature();
    }
    
    // ========================================
    // Query System
    // ========================================
    
    // Query entities with specific components
    template<typename... Components>
    std::vector<Entity> QueryEntities() const {
        // Build required signature
        Signature requiredSignature;
        (requiredSignature.set(GetComponentTypeID_Const<Components>()), ...);
        
        std::vector<Entity> result;
        for (const auto& [entity, signature] : m_signatures) {
            // Check if entity has all required components
            if ((signature & requiredSignature) == requiredSignature) {
                result.push_back(entity);
            }
        }
        return result;
    }
    
    // Check if entity matches signature
    bool EntityMatchesSignature(Entity entity, const Signature& requiredSignature) const {
        auto it = m_signatures.find(entity);
        if (it == m_signatures.end()) {
            return false;
        }
        return (it->second & requiredSignature) == requiredSignature;
    }
    
    // Helper to get the raw array for systems
    template<typename T>
    std::shared_ptr<ComponentArray<T>> GetComponentArray() const {
        std::type_index typeIndex(typeid(T));

        auto it = m_componentArrays.find(typeIndex);
        if (it == m_componentArrays.end()) {
            // Auto-create component array if not exists
            auto array = std::make_shared<ComponentArray<T>>();
            m_componentArrays[typeIndex] = array;
            return array;
        }

        return std::static_pointer_cast<ComponentArray<T>>(it->second);
    }

private:
    // Const version for query (doesn't modify m_componentTypeIDs)
    template<typename T>
    uint32_t GetComponentTypeID_Const() const {
        std::type_index typeIndex(typeid(T));
        auto it = m_componentTypeIDs.find(typeIndex);
        if (it == m_componentTypeIDs.end()) {
            throw std::runtime_error("Component type not registered");
        }
        return it->second;
    }
    
    // Event firing helpers
    void FireComponentAddedEvent(Entity entity, std::type_index componentType);
    void FireComponentRemovedEvent(Entity entity, std::type_index componentType);
    void FireEntityDestroyedEvent(Entity entity);

    // Entity ID generator
    EntityIDGenerator m_idGenerator;
    
    // Component type registration
    mutable std::unordered_map<std::type_index, uint32_t> m_componentTypeIDs;
    uint32_t m_nextComponentTypeID = 0;
    
    // Entity signatures (which components each entity has)
    std::unordered_map<Entity, Signature> m_signatures;
    
    // Map from type index to component array
    mutable std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> m_componentArrays;
    
    // Event bus for component lifecycle events
    EventBus* m_eventBus = nullptr;
};

} // namespace ECS
