#pragma once

#include <Windows.h>
#include <string>
#include <functional>
#include "../Events/Event.h"

class Window
{
public:
    Window();
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    void Initialize(
        HINSTANCE hInstance,
        int nCmdShow,
        const std::wstring& windowTitle,
        const std::wstring& windowClass,
        int width,
        int height
    );
    void SetEventCallback(const std::function<void(Event&)>& callback) { m_EventCallback = callback; }
    bool ProcessMessages();
    HWND GetHWND() const;

private:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    HWND m_hWnd;
    HINSTANCE m_hInstance;
    std::wstring m_windowTitle;
    std::wstring m_windowClass;
    int m_width;
    int m_height;
    
    std::function<void(Event&)> m_EventCallback;
};
