#pragma once

#include "Event.h"
#include <vector>
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <memory>

class EventBus
{
public:
    using EventCallbackFn = std::function<void(Event&)>;

    void Subscribe(EventType type, EventCallbackFn callback)
    {
        m_Subscribers[type].push_back(callback);
    }

    void Publish(Event& event)
    {
        auto& callbacks = m_Subscribers[event.GetEventType()];
        for (auto& callback : callbacks)
        {
            callback(event);
            if (event.Handled)
                break;
        }
    }

private:
    std::unordered_map<EventType, std::vector<EventCallbackFn>> m_Subscribers;
};
