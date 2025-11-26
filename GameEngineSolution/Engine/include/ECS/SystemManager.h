#pragma once

#include "System.h"
#include "SystemPhase.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <thread>
#include <future>

namespace ECS {

// ==================================================================================
// SystemManager
// ----------------------------------------------------------------------------------
// Manages the lifecycle and execution of all ECS systems.
// 
// Features:
// - Phase-based execution ordering
// - Parallel execution of thread-safe systems
// - System registration and retrieval
// - Automatic initialization and shutdown
// ==================================================================================
class SystemManager {
public:
    SystemManager() = default;
    
    ~SystemManager() {
        Shutdown();
    }

    // Register a new system
    // Register a new system
    template<typename T, typename... Args>
    T* AddSystem(ComponentManager& componentManager, Args&&... args) {
        auto system = std::make_unique<T>(componentManager, std::forward<Args>(args)...);
        T* systemPtr = system.get();
        
        // Set event bus if available (BEFORE Init so system can subscribe)
        if (m_eventBus) {
            systemPtr->SetEventBus(m_eventBus);
        }
        
        m_systems.push_back(std::move(system));
        systemPtr->Init();
        return systemPtr;
    }

    // Update all systems in phase order
    void Update(float deltaTime) {
        // Sort systems by phase if needed
        if (m_needsSort) {
            SortSystemsByPhase();
            m_needsSort = false;
        }
        
        // Execute each phase in order
        for (int phaseInt = static_cast<int>(SystemPhase::PreUpdate); 
             phaseInt <= static_cast<int>(SystemPhase::PreRender); 
             ++phaseInt) {
            
            SystemPhase phase = static_cast<SystemPhase>(phaseInt);
            UpdatePhase(phase, deltaTime);
        }
    }
    
    // Update a specific phase
    void UpdatePhase(SystemPhase phase, float deltaTime) {
        // Collect systems for this phase
        std::vector<System*> parallelSystems;
        std::vector<System*> sequentialSystems;
        
        for (auto& system : m_systems) {
            if (system->GetPhase() == phase) {
                if (system->CanParallelize()) {
                    parallelSystems.push_back(system.get());
                } else {
                    sequentialSystems.push_back(system.get());
                }
            }
        }
        
        // Execute parallel systems concurrently
        if (!parallelSystems.empty()) {
            std::vector<std::future<void>> futures;
            for (auto* system : parallelSystems) {
                futures.push_back(std::async(std::launch::async, [system, deltaTime]() {
                    system->Update(deltaTime);
                }));
            }
            // Wait for all parallel systems to complete
            for (auto& future : futures) {
                future.wait();
            }
        }
        
        // Execute sequential systems
        for (auto* system : sequentialSystems) {
            system->Update(deltaTime);
        }
    }

    // Get a specific system by type
    template<typename T>
    T* GetSystem() {
        for (auto& system : m_systems) {
            if (T* casted = dynamic_cast<T*>(system.get())) {
                return casted;
            }
        }
        return nullptr;
    }
    
    // Set event bus for all systems
    void SetEventBus(EventBus* eventBus) {
        m_eventBus = eventBus;
        for (auto& system : m_systems) {
            system->SetEventBus(eventBus);
        }
    }
    
    // Shutdown all systems
    void Shutdown() {
        for (auto& system : m_systems) {
            system->Shutdown();
        }
        m_systems.clear();
    }

private:
    void SortSystemsByPhase() {
        std::sort(m_systems.begin(), m_systems.end(),
            [](const std::unique_ptr<System>& a, const std::unique_ptr<System>& b) {
                return static_cast<int>(a->GetPhase()) < static_cast<int>(b->GetPhase());
            });
    }

    std::vector<std::unique_ptr<System>> m_systems;
    EventBus* m_eventBus = nullptr;
    bool m_needsSort = true;
};

} // namespace ECS

