Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET
{
    return Pixels.Sample(ClampSampler, uv);
}