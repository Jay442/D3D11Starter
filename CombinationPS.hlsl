// Texture resources
Texture2D TextureA : register(t0);
Texture2D TextureB : register(t1);
SamplerState BasicSampler : register(s0);

// Constant buffer (matches your existing color tint)
cbuffer ColorTint : register(b0)
{
    float4 colorTint;
};

// Vertex to pixel structure (matches your existing)
struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

// Main pixel shader function
float4 main(VertexToPixel input) : SV_TARGET
{
    // Sample both textures
    float4 texA = TextureA.Sample(BasicSampler, input.uv);
    float4 texB = TextureB.Sample(BasicSampler, input.uv);
    
    // Combine textures (example: multiply blend)
    float4 combined = texA * texB;
    
    // Apply existing color tint
    return combined * colorTint;
}