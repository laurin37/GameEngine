#include "../../include/Utils/EnginePCH.h"
#include "../../include/UI/ImmediateGUI.h"
#include "../../include/UI/UIRenderer.h"
#include "../../include/Input/Input.h"
#include "../../include/ResourceManagement/AssetManager.h"
#include "../../include/UI/SimpleFont.h"

ImmediateGUI::ImmediateGUI(UIRenderer* uiRenderer, Input* input, AssetManager* assetManager)
    : m_uiRenderer(uiRenderer), m_input(input), m_assetManager(assetManager)
{
}

ImmediateGUI::~ImmediateGUI() = default;

void ImmediateGUI::Initialize()
{
    // Any specific initialization if needed
}

void ImmediateGUI::BeginFrame()
{
    m_widgetCounter = 0;
    
    // Update input state
    m_state.mouseX = m_input->GetMouseX(); 
    m_state.mouseY = m_input->GetMouseY();
    
    m_state.mouseDown = m_input->IsMouseButtonDown(0); // Left click
    
    m_state.hotItem = 0;
}

void ImmediateGUI::EndFrame()
{
    if (!m_state.mouseDown)
    {
        m_state.activeItem = 0;
        if (m_captureItem != 0)
        {
            // Release capture if mouse released? Or let widget decide?
            // For buttons, yes. For sliders, maybe not until release.
        }
    }
}

void ImmediateGUI::Begin(const std::string& title, float x, float y, float width, float height)
{
    m_state.windowX = x;
    m_state.windowY = y;
    m_state.windowWidth = width;
    m_state.cursorX = x + m_style.WindowPadding;
    m_state.cursorY = y + m_style.WindowPadding + 20.0f; // Title bar height
    m_state.sameLine = false;

    // Draw Window Background
    SpriteVertex bgVertices[6];
    float x2 = x + width;
    float y2 = y + height;
    
    const float* bgCol = m_style.Colors[0]; // WindowBg

    // Quad for background
    bgVertices[0] = { {x, y, 0}, {0,0}, {bgCol[0], bgCol[1], bgCol[2], bgCol[3]} };
    bgVertices[1] = { {x2, y, 0}, {1,0}, {bgCol[0], bgCol[1], bgCol[2], bgCol[3]} };
    bgVertices[2] = { {x, y2, 0}, {0,1}, {bgCol[0], bgCol[1], bgCol[2], bgCol[3]} };
    bgVertices[3] = { {x, y2, 0}, {0,1}, {bgCol[0], bgCol[1], bgCol[2], bgCol[3]} };
    bgVertices[4] = { {x2, y, 0}, {1,0}, {bgCol[0], bgCol[1], bgCol[2], bgCol[3]} };
    bgVertices[5] = { {x2, y2, 0}, {1,1}, {bgCol[0], bgCol[1], bgCol[2], bgCol[3]} };
    
    m_uiRenderer->DrawSprite(bgVertices, 6, m_assetManager->GetWhiteTexture().Get());

    // Draw Title Bar
    float titleHeight = 25.0f;
    SpriteVertex titleVertices[6];
    float ty2 = y + titleHeight;
    
    const float* titleCol = m_style.Colors[1]; // TitleBg

    titleVertices[0] = { {x, y, 0}, {0,0}, {titleCol[0], titleCol[1], titleCol[2], titleCol[3]} };
    titleVertices[1] = { {x2, y, 0}, {1,0}, {titleCol[0], titleCol[1], titleCol[2], titleCol[3]} };
    titleVertices[2] = { {x, ty2, 0}, {0,1}, {titleCol[0], titleCol[1], titleCol[2], titleCol[3]} };
    titleVertices[3] = { {x, ty2, 0}, {0,1}, {titleCol[0], titleCol[1], titleCol[2], titleCol[3]} };
    titleVertices[4] = { {x2, y, 0}, {1,0}, {titleCol[0], titleCol[1], titleCol[2], titleCol[3]} };
    titleVertices[5] = { {x2, ty2, 0}, {1,1}, {titleCol[0], titleCol[1], titleCol[2], titleCol[3]} };
    
    m_uiRenderer->DrawSprite(titleVertices, 6, m_assetManager->GetWhiteTexture().Get());

    // Draw Title Text
    if (m_font)
    {
        m_uiRenderer->DrawString(*m_font, title, x + m_style.WindowPadding, y + 2.0f, 0.5f, m_style.Colors[2]);
    }

    // Clip content to window body (excluding title bar)
    PushClipRect(x, y + titleHeight, width, height - titleHeight);
}

void ImmediateGUI::End()
{
    PopClipRect();
}

void ImmediateGUI::SameLine()
{
    m_state.sameLine = true;
}

void ImmediateGUI::Label(const std::string& text)
{
    if (m_font)
    {
        m_uiRenderer->DrawString(*m_font, text, m_state.cursorX, m_state.cursorY, 0.4f, m_style.Colors[2]);
    }
    
    m_state.cursorY += ELEMENT_HEIGHT;
}

bool ImmediateGUI::Button(const std::string& text)
{
    int id = GetWidgetID();
    
    float x = m_state.cursorX;
    float y = m_state.cursorY;
    float w = m_state.windowWidth - (2 * m_style.WindowPadding); // Full width by default
    float h = ELEMENT_HEIGHT - 5.0f;
    
    // Handle SameLine
    if (m_state.sameLine)
    {
        x = m_state.cursorX + m_style.WindowPadding; // Add padding from previous element
        y = m_state.cursorY - ELEMENT_HEIGHT; // Stay on same line (undo Y advance)
        w = (m_state.windowWidth - (3 * m_style.WindowPadding)) * 0.5f; // Half width for now
        
        m_state.sameLine = false;
    }
    else
    {
        // Reset X to start of line
        x = m_state.windowX + m_style.WindowPadding;
    }

    // Update state cursor for drawing
    m_state.cursorX = x;
    m_state.cursorY = y;

    // Check interaction
    if (RegionHit(x, y, w, h))
    {
        m_state.hotItem = id;
        if (m_state.activeItem == 0 && m_state.mouseDown)
        {
            m_state.activeItem = id;
        }
    }

    // Render Button Background
    const float* color = m_style.Colors[3]; // Button Color
    if (m_state.hotItem == id)
    {
        // Simple lighten/darken for hover/active could be added here
        // For now, just use same color or maybe hardcode a slight variation
        if (m_state.activeItem == id) 
        {
             // Active
        }
        else 
        {
             // Hover
        }
    }

    SpriteVertex vertices[6];
    float x2 = x + w;
    float y2 = y + h;
    
    vertices[0] = { {x, y, 0}, {0,0}, {color[0], color[1], color[2], color[3]} };
    vertices[1] = { {x2, y, 0}, {1,0}, {color[0], color[1], color[2], color[3]} };
    vertices[2] = { {x, y2, 0}, {0,1}, {color[0], color[1], color[2], color[3]} };
    vertices[3] = { {x, y2, 0}, {0,1}, {color[0], color[1], color[2], color[3]} };
    vertices[4] = { {x2, y, 0}, {1,0}, {color[0], color[1], color[2], color[3]} };
    vertices[5] = { {x2, y2, 0}, {1,1}, {color[0], color[1], color[2], color[3]} };
    
    m_uiRenderer->DrawSprite(vertices, 6, m_assetManager->GetWhiteTexture().Get());

    // Render Button Text
    if (m_font)
    {
        // Center text using MeasureString
        DirectX::XMFLOAT2 textSize = m_font->MeasureString(text, 0.4f);
        float textX = x + (w - textSize.x) * 0.5f;
        float textY = y + (h - textSize.y) * 0.5f; // Center vertically too
        
        if (textX < x) textX = x;
        
        m_uiRenderer->DrawString(*m_font, text, textX, textY, 0.4f, m_style.Colors[2]);
    }

    // Advance cursor
    if (!m_state.sameLine) {
        m_state.cursorY += ELEMENT_HEIGHT;
        m_state.cursorX = m_state.windowX + m_style.WindowPadding; // Reset X
    } else {
        m_state.cursorX += w + m_style.WindowPadding;
    }

    // Click logic
    if (m_state.mouseDown == false && m_state.hotItem == id && m_state.activeItem == id)
    {
        return true;
    }

    return false;
}

bool ImmediateGUI::RegionHit(float x, float y, float w, float h)
{
    return (m_state.mouseX >= x && m_state.mouseX <= x + w &&
            m_state.mouseY >= y && m_state.mouseY <= y + h);
}

int ImmediateGUI::GetWidgetID()
{
    return ++m_widgetCounter;
}

void ImmediateGUI::SetCapture(int widgetID)
{
    m_captureItem = widgetID;
}

void ImmediateGUI::ReleaseCapture()
{
    m_captureItem = 0;
}

void ImmediateGUI::PushClipRect(float x, float y, float width, float height)
{
    m_uiRenderer->SetScissorRect(x, y, width, height);
}

void ImmediateGUI::PopClipRect()
{
    m_uiRenderer->SetScissorRect(0, 0, 0, 0); // Disable scissor
}
