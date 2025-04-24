// Include guard
#ifndef __GGP_LIGHTING__
#define __GGP_LIGHTING__

#define MAX_LIGHTS 128

#define MAX_SPECULAR_EXPONENT 256.0f

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

struct Light
{
    int Type;
    float3 Direction; // 16 bytes

    float Range;
    float3 Position; // 32 bytes

    float Intensity;
    float3 Color; // 48 bytes

    float SpotInnerAngle;
    float SpotOuterAngle;
    float2 Padding; // 64 bytes
};

// Lighting calculations
float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

float3 CalculateDiffuse(float3 normal, float3 lightDir)
{
    return saturate(dot(normal, lightDir));
}

float3 CalculateSpecular(float3 normal, float3 lightDir, float3 toCamera, float roughness, float3 diffuse)
{
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(toCamera, reflectDir), 0), (1 - roughness) * MAX_SPECULAR_EXPONENT);
    spec *= any(diffuse); 
    return spec;
}

float SpecularBlinnPhong(float3 normal, float3 dirToLight, float3 toCamera, float roughness)
{
	// Calculate halfway vector
    float3 halfwayVector = normalize(dirToLight + toCamera);

	// Compare halflway vector & normal and raise to a power
    return roughness == 1 ? 0.0f : pow(max(dot(halfwayVector, normal), 0), (1 - roughness) * MAX_SPECULAR_EXPONENT);
}

float3 CalculateDirectionalLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor, float specularScale)
{
    float3 lightDir = normalize(-light.Direction);
    float3 viewDir = normalize(camPos - worldPos);
    
    float3 diffuse = CalculateDiffuse(normal, lightDir);
    float3 specular = CalculateSpecular(normal, lightDir, viewDir, roughness, diffuse);
    return (diffuse * surfaceColor + specular) * light.Intensity * light.Color;
}

float3 CalculatePointLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor, float specularScale)
{
    float3 lightDir = normalize(light.Position - worldPos);
    float3 viewDir = normalize(camPos - worldPos);
    
    float attenuation = Attenuate(light, worldPos);
    float3 diffuse = CalculateDiffuse(normal, lightDir);
    float3 specular = CalculateSpecular(normal, lightDir, viewDir, roughness, diffuse);
    return (diffuse * surfaceColor + specular) * light.Intensity * light.Color;
}

float3 CalculateSpotLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float3 surfaceColor, float specularScale)
{
    float3 lightDir = normalize(light.Position - worldPos);
    
    float angle = saturate(dot(-lightDir, light.Direction));
    float cosOuter = cos(light.SpotOuterAngle);
    float cosInner = cos(light.SpotInnerAngle);
    float falloffRange = cosOuter - cosInner;
    float spotTerm = saturate((cosOuter - angle) / falloffRange);

    return CalculatePointLight(light, normal, worldPos, camPos, roughness, surfaceColor, specularScale) * spotTerm;
}

#endif