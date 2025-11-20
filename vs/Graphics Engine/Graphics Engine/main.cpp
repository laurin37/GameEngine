#include "Window.h"
#include "Graphics.h"
#include "Input.h"
#include <stdexcept>
#include <iostream>
#include <chrono>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    try
    {
        const int WINDOW_WIDTH = 1280;
        const int WINDOW_HEIGHT = 720;

        Window window;
        window.Initialize(hInstance, nCmdShow, L"GeminiDX Engine", L"GeminiDXWindowClass", WINDOW_WIDTH, WINDOW_HEIGHT);

        Graphics graphics;
        graphics.Initialize(window.GetHWND(), WINDOW_WIDTH, WINDOW_HEIGHT);
        
        Input input;
        input.Initialize(window.GetHWND());

        Camera* camera = graphics.GetCamera();

        auto lastTime = std::chrono::high_resolution_clock::now();

        while (true)
        {
            if (!window.ProcessMessages())
            {
                break; // Exit loop if WM_QUIT is received
            }

            input.Update();

            if (input.IsKeyDown(VK_ESCAPE))
            {
                break;
            }

            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
            lastTime = currentTime;

            const float moveSpeed = 5.0f * deltaTime;
            const float rotSpeed = 0.5f * deltaTime;

            if (input.IsKeyDown('W')) camera->AdjustPosition(0.0f, 0.0f, moveSpeed);
            if (input.IsKeyDown('S')) camera->AdjustPosition(0.0f, 0.0f, -moveSpeed);
            if (input.IsKeyDown('A')) camera->AdjustPosition(-moveSpeed, 0.0f, 0.0f);
            if (input.IsKeyDown('D')) camera->AdjustPosition(moveSpeed, 0.0f, 0.0f);
            if (input.IsKeyDown(VK_SPACE)) camera->AdjustPosition(0.0f, moveSpeed, 0.0f);
            if (input.IsKeyDown(VK_SHIFT)) camera->AdjustPosition(0.0f, -moveSpeed, 0.0f);

            float mouseDx = static_cast<float>(input.GetMouseDeltaX()) * rotSpeed;
            float mouseDy = static_cast<float>(input.GetMouseDeltaY()) * rotSpeed;
            camera->AdjustRotation(mouseDy, mouseDx, 0.0f);

            graphics.RenderFrame();
        }
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    return 0;
}
