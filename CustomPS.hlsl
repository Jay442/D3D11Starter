float random(float2 s)
{
    return frac(sin(dot(s, float2(12.9898, 78.233))) * 43758.5453123);
}

cbuffer Time : register(b1)
{
    float time;
};

struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

cbuffer ColorTint : register(b0)
{
    float4 colorTint;
};

float4 main(VertexToPixel input) : SV_TARGET
{
    float noise = random(input.uv + float2(time, 0));
    
    float4 finalColor = colorTint;
    
    finalColor.rgb *= noise;
    
    return finalColor;
}