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
    }
}

void ImmediateGUI::Begin(const std::string& title, float x, float y, float width, float height)
{
    m_state.windowX = x;
    m_state.windowY = y;
    m_state.windowWidth = width;
    m_state.cursorX = x + PADDING;
    m_state.cursorY = y + PADDING + 20.0f; // Title bar height
    m_state.sameLine = false;

    // Draw Window Background
    SpriteVertex bgVertices[6];
    float x2 = x + width;
    float y2 = y + height;
    
    // Quad for background
    bgVertices[0] = { {x, y, 0}, {0,0}, {WINDOW_BG_COLOR[0], WINDOW_BG_COLOR[1], WINDOW_BG_COLOR[2], WINDOW_BG_COLOR[3]} };
    bgVertices[1] = { {x2, y, 0}, {1,0}, {WINDOW_BG_COLOR[0], WINDOW_BG_COLOR[1], WINDOW_BG_COLOR[2], WINDOW_BG_COLOR[3]} };
    bgVertices[2] = { {x, y2, 0}, {0,1}, {WINDOW_BG_COLOR[0], WINDOW_BG_COLOR[1], WINDOW_BG_COLOR[2], WINDOW_BG_COLOR[3]} };
    bgVertices[3] = { {x, y2, 0}, {0,1}, {WINDOW_BG_COLOR[0], WINDOW_BG_COLOR[1], WINDOW_BG_COLOR[2], WINDOW_BG_COLOR[3]} };
    bgVertices[4] = { {x2, y, 0}, {1,0}, {WINDOW_BG_COLOR[0], WINDOW_BG_COLOR[1], WINDOW_BG_COLOR[2], WINDOW_BG_COLOR[3]} };
    bgVertices[5] = { {x2, y2, 0}, {1,1}, {WINDOW_BG_COLOR[0], WINDOW_BG_COLOR[1], WINDOW_BG_COLOR[2], WINDOW_BG_COLOR[3]} };
    
    m_uiRenderer->DrawSprite(bgVertices, 6, m_assetManager->GetWhiteTexture().Get());

    // Draw Title Bar
    float titleHeight = 25.0f;
    SpriteVertex titleVertices[6];
    float ty2 = y + titleHeight;
    
    titleVertices[0] = { {x, y, 0}, {0,0}, {TITLE_BG_COLOR[0], TITLE_BG_COLOR[1], TITLE_BG_COLOR[2], TITLE_BG_COLOR[3]} };
    titleVertices[1] = { {x2, y, 0}, {1,0}, {TITLE_BG_COLOR[0], TITLE_BG_COLOR[1], TITLE_BG_COLOR[2], TITLE_BG_COLOR[3]} };
    titleVertices[2] = { {x, ty2, 0}, {0,1}, {TITLE_BG_COLOR[0], TITLE_BG_COLOR[1], TITLE_BG_COLOR[2], TITLE_BG_COLOR[3]} };
    titleVertices[3] = { {x, ty2, 0}, {0,1}, {TITLE_BG_COLOR[0], TITLE_BG_COLOR[1], TITLE_BG_COLOR[2], TITLE_BG_COLOR[3]} };
    titleVertices[4] = { {x2, y, 0}, {1,0}, {TITLE_BG_COLOR[0], TITLE_BG_COLOR[1], TITLE_BG_COLOR[2], TITLE_BG_COLOR[3]} };
    titleVertices[5] = { {x2, ty2, 0}, {1,1}, {TITLE_BG_COLOR[0], TITLE_BG_COLOR[1], TITLE_BG_COLOR[2], TITLE_BG_COLOR[3]} };
    
    m_uiRenderer->DrawSprite(titleVertices, 6, m_assetManager->GetWhiteTexture().Get());

    // Draw Title Text
    if (m_font)
    {
        m_uiRenderer->DrawString(*m_font, title, x + PADDING, y + 2.0f, 0.5f, TEXT_COLOR);
    }
}

void ImmediateGUI::End()
{
    // Nothing to do for now
}

void ImmediateGUI::SameLine()
{
    m_state.sameLine = true;
}

void ImmediateGUI::Label(const std::string& text)
{
    if (m_font)
    {
        m_uiRenderer->DrawString(*m_font, text, m_state.cursorX, m_state.cursorY, 0.4f, TEXT_COLOR);
    }
    
    m_state.cursorY += ELEMENT_HEIGHT;
}

bool ImmediateGUI::Button(const std::string& text)
{
    int id = GetWidgetID();
    
    float x = m_state.cursorX;
    float y = m_state.cursorY;
    float w = m_state.windowWidth - (2 * PADDING); // Full width by default
    float h = ELEMENT_HEIGHT - 5.0f;
    
    // Handle SameLine (simplified)
    if (m_state.sameLine)
    {
        // TODO: Implement proper layout calculation
        // For now, just reset sameLine
        m_state.sameLine = false;
    }

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
    const float* color = BUTTON_COLOR;
    if (m_state.hotItem == id)
    {
        color = (m_state.activeItem == id) ? BUTTON_ACTIVE_COLOR : BUTTON_HOT_COLOR;
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
        // Center text (approximate)
        float textWidth = text.length() * 10.0f; // Hacky width estimation
        float textX = x + (w - textWidth) * 0.5f;
        if (textX < x) textX = x;
        
        m_uiRenderer->DrawString(*m_font, text, x + PADDING, y + 5.0f, 0.4f, TEXT_COLOR);
    }

    // Advance cursor
    m_state.cursorY += ELEMENT_HEIGHT;

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
