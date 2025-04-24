// Include guard
#ifndef __GGP_LIGHTING__
#define __GGP_LIGHTING__

#define MAX_LIGHTS 128

#define MAX_SPECULAR_EXPONENT 256.0f

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

// A constant Fresnel value for non-metals (glass and plastic have values of about 0.04)
static const float F0_NON_METAL = 0.04f;

// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal

static const float PI = 3.14159265359f;

struct Light
{
    int Type;
    float3 Direction; // 16 bytes

    float Range;
    float3 Position; // 32 bytes

    float Intensity;
    float3 Color; // 48 bytes

    float SpotFalloff;
    float3 Padding; // 64 bytes
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

// Calculates diffuse amount based on energy conservation
//
// diffuse   - Diffuse amount
// F         - Fresnel result from microfacet BRDF
// metalness - surface metalness amount 
float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}

// Normal Distribution Function: GGX (Trowbridge-Reitz)
//
// a - Roughness
// h - Half vector
// n - Normal
// 
// D(h, n, a) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float D_GGX(float3 n, float3 h, float roughness)
{
	// Pre-calculations
    float NdotH = saturate(dot(n, h));
    float NdotH2 = NdotH * NdotH;
    float a = roughness * roughness;
    float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

	// ((n dot h)^2 * (a^2 - 1) + 1)
	// Can go to zero if roughness is 0 and NdotH is 1
    float denomToSquare = NdotH2 * (a2 - 1) + 1;

	// Final value
    return a2 / (PI * denomToSquare * denomToSquare);
}

// Fresnel term - Schlick approx.
// 
// v - View vector
// h - Half vector
// f0 - Value when l = n
//
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 F_Schlick(float3 v, float3 h, float3 f0)
{
	// Pre-calculations
    float VdotH = saturate(dot(v, h));

	// Final value
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}



// Geometric Shadowing - Schlick-GGX
// - k is remapped to a / 2, roughness remapped to (r+1)/2 before squaring!
//
// n - Normal
// v - View vector
//
// G_Schlick(n,v,a) = (n dot v) / ((n dot v) * (1 - k) * k)
//
// Full G(n,v,l,a) term = G_SchlickGGX(n,v,a) * G_SchlickGGX(n,l,a)
float G_SchlickGGX(float3 n, float3 v, float roughness)
{
	// End result of remapping:
    float k = pow(roughness + 1, 2) / 8.0f;
    float NdotV = saturate(dot(n, v));

	// Final value
	// Note: Numerator should be NdotV (or NdotL depending on parameters).
	// However, these are also in the BRDF's denominator, so they'll cancel!
	// We're leaving them out here AND in the BRDF function as the
	// dot products can get VERY small and cause rounding errors.
    return 1 / (NdotV * (1 - k) + k);
}


 
// Cook-Torrance Microfacet BRDF (Specular)
//
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - parts of the denominator are canceled out by numerator (see below)
//
// D() - Normal Distribution Function - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 f0, out float3 F_out)
{
	// Other vectors
    float3 h = normalize(v + l);

	// Run numerator functions
    float D = D_GGX(n, h, roughness);
    float3 F = F_Schlick(v, h, f0);
    float G = G_SchlickGGX(n, v, roughness) * G_SchlickGGX(n, l, roughness);
	
	// Pass F out of the function for diffuse balance
    F_out = F;

	// Final specular formula
	// Note: The denominator SHOULD contain (NdotV)(NdotL), but they'd be
	// canceled out by our G() term.  As such, they have been removed
	// from BOTH places to prevent floating point rounding errors.
    float3 specularResult = (D * F * G) / 4;

	// One last non-obvious requirement: According to the rendering equation,
	// specular must have the same NdotL applied as diffuse!  We'll apply
	// that here so that minimal changes are required elsewhere.
    return specularResult * max(dot(n, l), 0);
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

float3 CalculateDirectionalLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float metalness, float3 surfaceColor, float3 specularColor)
{
    float3 lightDir = normalize(-light.Direction);
    float3 viewDir = normalize(camPos - worldPos);
    
    float3 diffuse = CalculateDiffuse(normal, lightDir);
    float3 F;
    float3 specular = MicrofacetBRDF(normal, lightDir, viewDir, roughness, specularColor, F);
    
    float3 balancedDiffuse = DiffuseEnergyConserve(diffuse, specular, metalness);
    return (balancedDiffuse * surfaceColor + specular) * light.Intensity * light.Color;
}

float3 CalculatePointLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float metalness, float3 surfaceColor, float3 specularColor)
{
    float3 lightDir = normalize(light.Position - worldPos);
    float3 viewDir = normalize(camPos - worldPos);
    
    float attenuation = Attenuate(light, worldPos);
    float3 diffuse = CalculateDiffuse(normal, lightDir);
    float3 F;
    float3 specular = MicrofacetBRDF(normal, lightDir, viewDir, roughness, specularColor, F);
    
    float3 balancedDiffuse = DiffuseEnergyConserve(diffuse, specular, metalness);
    return (balancedDiffuse * surfaceColor + specular) * attenuation * light.Intensity * light.Color;
}

float3 CalculateSpotLight(Light light, float3 normal, float3 worldPos, float3 camPos, float roughness, float metalness, float3 surfaceColor, float3 specularColor)
{
    float3 lightDir = normalize(light.Position - worldPos);
    float penumbra = pow(saturate(dot(-lightDir, light.Direction)), light.SpotFalloff);


    return CalculatePointLight(light, normal, worldPos, camPos, roughness, metalness, surfaceColor, specularColor) * penumbra;
}

#endif