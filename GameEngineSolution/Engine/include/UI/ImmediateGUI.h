#pragma once

#include <string>
#include <vector>
#include <memory>
#include <DirectXMath.h>

// Forward declarations
class UIRenderer;
class Input;
class AssetManager;
class SimpleFont;

// ==================================================================================
// ImmediateGUI
// ----------------------------------------------------------------------------------
// A lightweight Immediate Mode GUI system.
// Allows creating UI elements (Buttons, Labels, etc.) in code without retaining state.
// 
// Usage:
//   gui->BeginFrame();
//   gui->Begin("My Window", 10, 10, 200, 300);
//   if (gui->Button("Click Me")) { ... }
//   gui->End();
//   gui->EndFrame();
// ==================================================================================
class ImmediateGUI
{
public:
    ImmediateGUI(UIRenderer* uiRenderer, Input* input, AssetManager* assetManager);
    ~ImmediateGUI();

    void Initialize();

    // Frame Management
    void BeginFrame();
    void EndFrame();

    // Window/Layout
    void Begin(const std::string& title, float x, float y, float width, float height);
    void End();
    void SameLine();

    // Widgets
    void Label(const std::string& text);
    bool Button(const std::string& text);
    
    // Styling
    void SetFont(const SimpleFont* font) { m_font = font; }

    // Advanced Features
    void PushClipRect(float x, float y, float width, float height);
    void PopClipRect();
    
    // Input Capture
    void SetCapture(int widgetID);
    void ReleaseCapture();
    int GetCaptureID() const { return m_captureItem; }

private:
    struct UIState
    {
        int hotItem = 0;    // Item currently hovered
        int activeItem = 0; // Item currently being clicked
        int mouseX = 0;
        int mouseY = 0;
        bool mouseDown = false;
        
        // Layout state
        float cursorX = 0;
        float cursorY = 0;
        float windowX = 0;
        float windowY = 0;
        float windowWidth = 0;
        float nextItemX = 0; // For SameLine()
        bool sameLine = false;
    };

    // Styling
    struct UIStyle
    {
        float WindowPadding = 10.0f;
        float ItemSpacing = 5.0f;
        float Colors[4][4] = {
            {0.2f, 0.2f, 0.2f, 0.9f}, // WindowBg
            {0.3f, 0.3f, 0.3f, 1.0f}, // TitleBg
            {1.0f, 1.0f, 1.0f, 1.0f}, // Text
            {0.4f, 0.4f, 0.4f, 1.0f}  // Button
        };
        // Add more colors as needed (Hover, Active, etc.)
    };
    
    UIStyle& GetStyle() { return m_style; }

private:
    // Helpers
    bool RegionHit(float x, float y, float w, float h);
    int GetWidgetID(); // Generate unique ID based on call order/hash

    UIRenderer* m_uiRenderer;
    Input* m_input;
    AssetManager* m_assetManager;
    const SimpleFont* m_font = nullptr;

    struct State
    {
        float mouseX, mouseY;
        bool mouseDown;
        
        int hotItem;
        int activeItem;
        
        float cursorX, cursorY;
        float windowX, windowY;
        float windowWidth;
        bool sameLine;
    } m_state;

    int m_widgetCounter = 0; // Reset every frame
    int m_captureItem = 0; // Item that has captured input
    UIStyle m_style;
    
    // Hardcoded constants removed in favor of Style
    // const float PADDING = 8.0f;
    const float ELEMENT_HEIGHT = 30.0f;
    // const float BUTTON_COLOR[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
    // const float BUTTON_HOT_COLOR[4] = { 0.4f, 0.4f, 0.4f, 1.0f };
    // const float BUTTON_ACTIVE_COLOR[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    // const float TEXT_COLOR[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    // const float WINDOW_BG_COLOR[4] = { 0.1f, 0.1f, 0.1f, 0.9f };
    // const float TITLE_BG_COLOR[4] = { 0.0f, 0.4f, 0.8f, 1.0f };
};
