// Shaders/PostProcess.hlsl

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

// Full-screen triangle vertex shader trick
// No vertex buffer needed!
VS_OUTPUT VS_main(uint vertexID : SV_VertexID)
{
    VS_OUTPUT output;
    // Map vertex ID to UV coordinates and clip space positions
    // This generates a triangle that covers the entire screen
    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.pos = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);
    // Flip the V coordinate
    output.pos.y = -output.pos.y;
    return output;
}


Texture2D sceneTexture : register(t0);
Texture2D bloomTexture : register(t1);
SamplerState sceneSampler : register(s0);

// ACES Filmic Tonemapping Curve
// Source: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float4 PS_main(VS_OUTPUT input) : SV_TARGET
{
    // === DEBUG: MAKE THIS SUPER OBVIOUS ===
    // The entire screen should have a BLUE TINT if this shader is running
    
    // 1. Sample the scene texture
    float3 color = sceneTexture.Sample(sceneSampler, input.uv).rgb;
    
    // 2. Add OBVIOUS blue tint to prove this shader is running
    color += float3(0.0, 0.0, 0.5); // Add blue
    
    // 3. Sample bloom texture
    float3 bloom = bloomTexture.Sample(sceneSampler, input.uv).rgb;
    
    // If bloom has any value, make screen turn YELLOW instead
    float bloomIntensity = length(bloom);
    if (bloomIntensity > 0.001)
    {
        color = float3(1, 1, 0); // BRIGHT YELLOW = bloom texture has data
    }
    
    // Apply Gamma Correction
    color = pow(color, 1.0f / 2.2f);
    
    return float4(color, 1.0f);
}
