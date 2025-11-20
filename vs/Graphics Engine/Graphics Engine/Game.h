#pragma once

#include "Window.h"
#include "Graphics.h"
#include "Input.h"
#include "Camera.h"
#include "GameObject.h"
#include <memory>
#include <vector>
#include <chrono>

class Game
{
public:
    Game();
    ~Game() = default;

    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    void Run();

private:
    void Update(float deltaTime);
    void Render();

    Window m_window;
    Graphics m_graphics;
    Input m_input;

    std::unique_ptr<Camera> m_camera;
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    
    std::chrono::steady_clock::time_point m_lastTime;
};
