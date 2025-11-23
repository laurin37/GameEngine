// BrightPass.hlsl
// Extracts bright pixels for bloom effect

Texture2D sceneTexture : register(t0);
SamplerState sceneSampler : register(s0);

cbuffer BlurParams : register(b0)
{
    float2 direction;   // Not used in bright pass
    float threshold;    // Brightness threshold
    float padding;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    // DEBUG: Always return green to test if this shader is even running
    return float4(0.0, 1.0, 0.0, 1.0); // BRIGHT GREEN
    
    // Original code (commented out for testing):
    /*
    float3 color = sceneTexture.Sample(sceneSampler, input.uv).rgb;
    float brightness = dot(color, float3(0.2126, 0.7152, 0.0722));
    float contribution = saturate((brightness - threshold) / threshold);
    return float4(color * contribution, 1.0);
    */
}
