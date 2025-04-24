#include "ShaderStructs.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix view;
    matrix projection;
}

VertexToPixel_Sky main(VertexShaderInput input)
{
    VertexToPixel_Sky output;
    
    matrix viewNoTranslation = view;
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;
    
    matrix projectedView = mul(projection, viewNoTranslation);
    output.screenPosition = mul(projectedView, float4(input.localPosition, 1.0f));
    
    output.screenPosition.z = output.screenPosition.w;
    output.sampleDir = input.localPosition;
    return output;
}