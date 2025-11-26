#pragma once

#include "Event.h"
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <algorithm>
#include <string>

// Event priority levels (lower number = higher priority)
enum class EventPriority 
{
    High = 0,
    Normal = 1,
    Low = 2
};

// Forward declaration
class EventBus;

// RAII guard for automatic unsubscription
class SubscriptionGuard
{
public:
    SubscriptionGuard(EventBus* bus, EventType type, size_t id)
        : m_bus(bus), m_type(type), m_id(id) {}
    
    ~SubscriptionGuard();
    
    SubscriptionGuard(const SubscriptionGuard&) = delete;
    SubscriptionGuard& operator=(const SubscriptionGuard&) = delete;
    SubscriptionGuard(SubscriptionGuard&& other) noexcept
        : m_bus(other.m_bus), m_type(other.m_type), m_id(other.m_id)
    {
        other.m_bus = nullptr;
    }
    
    size_t GetId() const { return m_id; }
    
private:
    EventBus* m_bus;
    EventType m_type;
    size_t m_id;
};

// Event statistics for profiling
struct EventStats
{
    size_t totalPublished = 0;
    size_t totalHandled = 0;
    std::unordered_map<EventType, size_t> countByType;
    
    void Reset() 
    {
        totalPublished = 0;
        totalHandled = 0;
        countByType.clear();
    }
};

class EventBus
{
public:
    using EventCallbackFn = std::function<void(Event&)>;
    using SubscriptionId = size_t;

    EventBus() : m_nextId(0), m_debugMode(false) {}

    // Subscribe to a specific event type with priority
    SubscriptionId Subscribe(EventType type, EventCallbackFn callback, EventPriority priority = EventPriority::Normal)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        SubscriptionId id = m_nextId++;
        m_subscribers[type][static_cast<int>(priority)].push_back({id, std::move(callback)});
        
        if (m_debugMode) {
            LogDebug("Subscribed to " + GetEventTypeName(type) + 
                    " (Priority: " + GetPriorityName(priority) + ", ID: " + std::to_string(id) + ")");
        }
        
        return id;
    }

    // Subscribe with RAII guard for automatic cleanup
    std::unique_ptr<SubscriptionGuard> SubscribeGuarded(EventType type, EventCallbackFn callback, EventPriority priority = EventPriority::Normal)
    {
        SubscriptionId id = Subscribe(type, std::move(callback), priority);
        return std::make_unique<SubscriptionGuard>(this, type, id);
    }

    // Subscribe to all events in a category
    SubscriptionId SubscribeByCategory(int categoryFlags, EventCallbackFn callback, EventPriority priority = EventPriority::Normal)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        SubscriptionId id = m_nextId++;
        m_categorySubscribers[static_cast<int>(priority)].push_back({id, categoryFlags, std::move(callback)});
        
        if (m_debugMode) {
            LogDebug("Subscribed to category " + std::to_string(categoryFlags) + 
                    " (Priority: " + GetPriorityName(priority) + ", ID: " + std::to_string(id) + ")");
        }
        
        return id;
    }

    // Unsubscribe from a specific event type
    void Unsubscribe(EventType type, SubscriptionId id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto& priorityMap = m_subscribers[type];
        for (auto& [priority, subscriptions] : priorityMap) {
            auto it = std::remove_if(subscriptions.begin(), subscriptions.end(),
                [id](const Subscription& sub) { return sub.id == id; });
            subscriptions.erase(it, subscriptions.end());
        }
        
        if (m_debugMode) {
            LogDebug("Unsubscribed from " + GetEventTypeName(type) + " (ID: " + std::to_string(id) + ")");
        }
    }

    // Unsubscribe from category subscription
    void UnsubscribeCategory(SubscriptionId id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto& [priority, subscriptions] : m_categorySubscribers) {
            auto it = std::remove_if(subscriptions.begin(), subscriptions.end(),
                [id](const CategorySubscription& sub) { return sub.id == id; });
            subscriptions.erase(it, subscriptions.end());
        }
        
        if (m_debugMode) {
            LogDebug("Unsubscribed from category (ID: " + std::to_string(id) + ")");
        }
    }

    // Queue an event for deferred processing
    void QueueEvent(std::unique_ptr<Event> event)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_debugMode) {
            LogDebug("Queued event: " + std::string(event->GetName()));
        }
        m_eventQueue.push_back(std::move(event));
    }

    // Publish an event immediately (synchronous)
    void Publish(Event& event)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_stats.totalPublished++;
        m_stats.countByType[event.GetEventType()]++;
        
        if (m_debugMode) {
            LogDebug("Publishing: " + std::string(event.GetName()));
        }
        
        // Process type-specific subscribers (High -> Normal -> Low)
        auto& priorityMap = m_subscribers[event.GetEventType()];
        for (int p = static_cast<int>(EventPriority::High); 
             p <= static_cast<int>(EventPriority::Low); 
             ++p) 
        {
            auto it = priorityMap.find(p);
            if (it != priorityMap.end()) {
                for (auto& subscription : it->second) {
                    subscription.callback(event);
                    if (event.Handled) {
                        m_stats.totalHandled++;
                        if (m_debugMode) {
                            LogDebug("  -> Handled by subscriber (ID: " + std::to_string(subscription.id) + ")");
                        }
                        return;
                    }
                }
            }
        }
        
        // Process category subscribers (High -> Normal -> Low)
        int categoryFlags = event.GetCategoryFlags();
        for (int p = static_cast<int>(EventPriority::High); 
             p <= static_cast<int>(EventPriority::Low); 
             ++p) 
        {
            auto it = m_categorySubscribers.find(p);
            if (it != m_categorySubscribers.end()) {
                for (auto& subscription : it->second) {
                    if (categoryFlags & subscription.categoryFlags) {
                        subscription.callback(event);
                        if (event.Handled) {
                            m_stats.totalHandled++;
                            if (m_debugMode) {
                                LogDebug("  -> Handled by category subscriber (ID: " + std::to_string(subscription.id) + ")");
                            }
                            return;
                        }
                    }
                }
            }
        }
    }

    // Process all queued events
    void ProcessEvents()
    {
        std::vector<std::unique_ptr<Event>> eventsToProcess;
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            eventsToProcess = std::move(m_eventQueue);
            m_eventQueue.clear();
        }
        
        if (m_debugMode && !eventsToProcess.empty()) {
            LogDebug("Processing " + std::to_string(eventsToProcess.size()) + " queued events");
        }
        
        for (auto& event : eventsToProcess) {
            Publish(*event);
        }
    }

    // Enable/disable debug logging
    void SetDebugMode(bool enabled) 
    { 
        std::lock_guard<std::mutex> lock(m_mutex);
        m_debugMode = enabled;
        if (enabled) {
            LogDebug("EventBus debug mode enabled");
        }
    }

    // Get event statistics
    EventStats GetStats() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    // Reset statistics
    void ResetStats()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.Reset();
        if (m_debugMode) {
            LogDebug("Statistics reset");
        }
    }

    // Get subscriber count for debugging
    size_t GetSubscriberCount(EventType type) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        size_t count = 0;
        auto it = m_subscribers.find(type);
        if (it != m_subscribers.end()) {
            for (const auto& [priority, subscriptions] : it->second) {
                count += subscriptions.size();
            }
        }
        return count;
    }

private:
    struct Subscription
    {
        SubscriptionId id;
        EventCallbackFn callback;
    };

    struct CategorySubscription
    {
        SubscriptionId id;
        int categoryFlags;
        EventCallbackFn callback;
    };

    // Helper to get event type name
    static std::string GetEventTypeName(EventType type)
    {
        switch (type) {
            case EventType::None: return "None";
            case EventType::WindowClose: return "WindowClose";
            case EventType::WindowResize: return "WindowResize";
            case EventType::WindowFocus: return "WindowFocus";
            case EventType::WindowLostFocus: return "WindowLostFocus";
            case EventType::KeyPressed: return "KeyPressed";
            case EventType::KeyReleased: return "KeyReleased";
            case EventType::KeyTyped: return "KeyTyped";
            case EventType::MouseButtonPressed: return "MouseButtonPressed";
            case EventType::MouseButtonReleased: return "MouseButtonReleased";
            case EventType::MouseMoved: return "MouseMoved";
            case EventType::MouseScrolled: return "MouseScrolled";
            default: return "Unknown";
        }
    }

    // Helper to get priority name
    static std::string GetPriorityName(EventPriority priority)
    {
        switch (priority) {
            case EventPriority::High: return "High";
            case EventPriority::Normal: return "Normal";
            case EventPriority::Low: return "Low";
            default: return "Unknown";
        }
    }

    // Debug logging (can be hooked to actual logger)
    void LogDebug(const std::string& message) const
    {
        // This will be picked up by Logger if available
        // For now we'll use a simple approach that can be replaced
        #ifdef _DEBUG
        // Could call LOG_INFO here if available
        // For now, no-op as we have the debug mode flag
        #endif
    }

    mutable std::mutex m_mutex;
    SubscriptionId m_nextId;
    bool m_debugMode;
    EventStats m_stats;
    
    // EventType -> Priority -> Subscriptions
    std::unordered_map<EventType, std::unordered_map<int, std::vector<Subscription>>> m_subscribers;
    
    // Priority -> Category Subscriptions
    std::unordered_map<int, std::vector<CategorySubscription>> m_categorySubscribers;
    
    // Event queue for deferred processing
    std::vector<std::unique_ptr<Event>> m_eventQueue;
};

// SubscriptionGuard destructor implementation
inline SubscriptionGuard::~SubscriptionGuard()
{
    if (m_bus) {
        m_bus->Unsubscribe(m_type, m_id);
    }
}
