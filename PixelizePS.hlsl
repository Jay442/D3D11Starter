Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

cbuffer PixelizeSettings : register(b0)
{
    float2 pixelSize;
    float2 screenSize; 
};

float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET
{
    float2 pixelUV = uv * screenSize;
    pixelUV = floor(pixelUV / pixelSize) * pixelSize;
    pixelUV /= screenSize;

    return Pixels.Sample(ClampSampler, pixelUV);
}