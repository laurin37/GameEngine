#pragma once

#include <unordered_map>
#include <windows.h>
#include <functional>
#include "../Events/Event.h"

enum class Action {
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    Jump,
    Fire,
    AltFire,
    Reload,
    Quit,
    None
};

class Input
{
public:
    Input();
    ~Input() = default;

    struct MouseState
    {
        int x, y;
        int dx, dy;
    };

    void Initialize(HWND hwnd);
    void SetEventCallback(const std::function<void(Event&)>& callback) { m_EventCallback = callback; }
    void Update();

    bool IsKeyDown(int key) const;
    bool IsMouseButtonDown(int button) const;
    
    // Action Mapping
    void BindAction(Action action, int key);
    bool IsActionDown(Action action) const;

    int GetMouseDeltaX() const;
    int GetMouseDeltaY() const;

private:
    HWND m_hwnd;
    bool m_keys[256];
    MouseState m_mouseState;
    std::unordered_map<Action, int> m_actionBindings;
    std::function<void(Event&)> m_EventCallback;
};
