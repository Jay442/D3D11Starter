#include "ShaderStructs.hlsli"
#include "Lighting.hlsli"

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
SamplerState BasicSampler : register(s0); // "s" registers for samplers
Texture2D SpecularMap : register(t1);


cbuffer ExternalData : register(b0)
{
    Light lights[MAX_LIGHTS];
    int lightCount;
    float3 ambientColor;
    
    float3 cameraPosition;
    
    float4 colorTint;
    float roughness;
    float2 uvScale;
    float2 uvOffset;
};

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    input.normal = normalize(input.normal);
    input.uv = input.uv * uvScale + uvOffset;
    
    float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb * colorTint.rgb;
    
    float specularScale = SpecularMap.Sample(BasicSampler, input.uv).r;
    
    float3 viewDir = normalize(cameraPosition - input.worldPos);
    
    float3 totalLight = ambientColor * surfaceColor;
    
    // Process each light
    for (int i = 0; i < lightCount; i++)
    {
        Light light = lights[i];
        float3 lightDir = normalize(light.Direction);
        
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                totalLight += CalculateDirectionalLight(light, input.normal, input.worldPos, cameraPosition, roughness, surfaceColor, specularScale);
                break;

            case LIGHT_TYPE_POINT:
                totalLight += CalculatePointLight(light, input.normal, input.worldPos, cameraPosition, roughness, surfaceColor, specularScale);
                break;

            case LIGHT_TYPE_SPOT:
                totalLight += CalculateSpotLight(light, input.normal, input.worldPos, cameraPosition, roughness, surfaceColor, specularScale);
                break;
        }
    }

    return float4(totalLight, 1);

}