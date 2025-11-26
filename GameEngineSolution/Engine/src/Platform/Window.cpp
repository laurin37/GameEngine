#include <stdexcept>

#include "../../include/Platform/Window.h"
#include "../../include/Events/ApplicationEvents.h"
#include "../../include/Events/InputEvents.h"
#include "../../include/Events/EventBus.h"
#include "../../include/Utils/Logger.h"

Window::Window()
    : m_hWnd(nullptr), m_hInstance(nullptr), m_width(0), m_height(0)
{
}

Window::~Window()
{
    if (m_hInstance)
    {
        UnregisterClassW(m_windowClass.c_str(), m_hInstance);
    }
}

void Window::Initialize(
    HINSTANCE hInstance,
    int nCmdShow,
    const std::wstring& windowTitle,
    const std::wstring& windowClass,
    int width,
    int height)
{
    m_hInstance = hInstance;
    m_windowTitle = windowTitle;
    m_windowClass = windowClass;
    m_width = width;
    m_height = height;

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = Window::WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = m_windowClass.c_str();
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassExW(&wc))
    {
        throw std::runtime_error("Failed to register window class.");
    }

    RECT wr = { 0, 0, width, height };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    m_hWnd = CreateWindowExW(
        0,
        m_windowClass.c_str(),
        m_windowTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left,
        wr.bottom - wr.top,
        nullptr,
        nullptr,
        m_hInstance,
        this
    );

    if (!m_hWnd)
    {
        throw std::runtime_error("Failed to create window.");
    }

    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    ShowCursor(FALSE);
}

bool Window::ProcessMessages()
{
    MSG msg = {};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return true;
}

HWND Window::GetHWND() const
{
    return m_hWnd;
}

LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Window* pWindow = nullptr;

    if (message == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pWindow = reinterpret_cast<Window*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)pWindow);
        pWindow->m_hWnd = hWnd;
    }
    else
    {
        pWindow = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    if (pWindow)
    {
        switch (message)
        {
        case WM_CLOSE:
        {
            WindowCloseEvent event;
            if (pWindow->m_eventBus) pWindow->m_eventBus->Publish(event);
            DestroyWindow(hWnd);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            pWindow->m_width = width;
            pWindow->m_height = height;
            
            WindowResizeEvent event(width, height);
            if (pWindow->m_eventBus) pWindow->m_eventBus->Publish(event);
            return 0;
        }
        case WM_KEYDOWN:
        {
            int keycode = static_cast<int>(wParam);
            int repeatCount = lParam & 0xFFFF;
            KeyPressedEvent event(keycode, repeatCount);
            if (pWindow->m_eventBus) pWindow->m_eventBus->Publish(event);
            return 0;
        }
        case WM_KEYUP:
        {
            int keycode = static_cast<int>(wParam);
            KeyReleasedEvent event(keycode);
            if (pWindow->m_eventBus) pWindow->m_eventBus->Publish(event);
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            MouseMovedEvent event((float)x, (float)y);
            if (pWindow->m_eventBus) pWindow->m_eventBus->Publish(event);
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        {
            int button = 0;
            if (message == WM_LBUTTONDOWN) button = VK_LBUTTON;
            else if (message == WM_RBUTTONDOWN) button = VK_RBUTTON;
            else if (message == WM_MBUTTONDOWN) button = VK_MBUTTON;

            MouseButtonPressedEvent event(button);
            if (pWindow->m_eventBus) pWindow->m_eventBus->Publish(event);
            return 0;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            int button = 0;
            if (message == WM_LBUTTONUP) button = VK_LBUTTON;
            else if (message == WM_RBUTTONUP) button = VK_RBUTTON;
            else if (message == WM_MBUTTONUP) button = VK_MBUTTON;

            MouseButtonReleasedEvent event(button);
            if (pWindow->m_eventBus) pWindow->m_eventBus->Publish(event);
            return 0;
        }
        }
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}
