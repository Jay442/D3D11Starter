#include "ShaderStructs.hlsli"
#include "Lighting.hlsli"

SamplerState BasicSampler : register(s0); // "s" registers for samplers
Texture2D Albedo : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);

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
    
    // Normal Maping
    float3 normalMapSample = NormalMap.Sample(BasicSampler, input.uv).rgb;
    float3 unpackedNormal = normalize(normalMapSample * 2.0f - 1.0f);
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent);
    
    // Orthogonalize tangent with Gram-Schmidt
    T = normalize(T - dot(T, N) * N);
    
    // Calculate bitangent
    float3 B = cross(N, T);
    
    // Create TBN matrix
    float3x3 TBN = float3x3(T, B, N);
    
    // Transform normal from tangent to world space
    float3 finalNormal = mul(unpackedNormal, TBN);
    finalNormal = normalize(finalNormal);
    
    float3 surfaceColor = pow(Albedo.Sample(BasicSampler, input.uv).rgb * colorTint.rgb, 2.2f);
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;
    
    // Specular color determination -----------------
    // Assume albedo texture is actually holding specular color where metalness == 1
    // Note the use of lerp here - metal is generally 0 or 1, but might be in between
    // because of linear texture sampling, so we lerp the specular color to match
    float3 specularColor = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);
    
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
                totalLight += CalculateDirectionalLight(light, input.normal, input.worldPos, cameraPosition, roughness, metalness, surfaceColor, specularColor);
                break;

            case LIGHT_TYPE_POINT:
                totalLight += CalculatePointLight(light, input.normal, input.worldPos, cameraPosition, roughness, metalness, surfaceColor, specularColor);
                break;

            case LIGHT_TYPE_SPOT:
                totalLight += CalculateSpotLight(light, input.normal, input.worldPos, cameraPosition, roughness, metalness, surfaceColor, specularColor);
                break;
        }
    }

    return float4(pow(totalLight, 1.0f / 2.2f), 1);

}