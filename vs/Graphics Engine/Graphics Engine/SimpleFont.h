#pragma once

#include "Graphics.h"
#include <string>
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>

class SimpleFont
{
public:
    SimpleFont();
    ~SimpleFont() = default;

    void Initialize(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> fontTexture);
    void DrawString(Graphics& gfx, const std::string& text, float x, float y, float size, float color[4]);

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_fontTexture;
    static const int MAX_CHARS = 256; // Max chars per draw call for this simple buffer
    SpriteVertex m_spriteBuffer[MAX_CHARS * 6];
};