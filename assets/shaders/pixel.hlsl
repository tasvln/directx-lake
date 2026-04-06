// I do realise i can't be perfect with shaders in 1 day/week and it'd take a bit for experience
//  to master or somewhat fully understand what i'm doing but glad i understand the math aspect of
//  things... that being said, i need more experience and need to read up more

#include "lighting.hlsl"

struct Material
{
    float4 emissive;       // self-lit color
    float4 ambient;        // base ambient color
    float4 diffuse;        // diffuse reflectance
    float4 specular;       // specular reflectance
    float  specularPower;  // shininess exponent
    float  useTexture;     // 1 = use diffuse texture
    float  useNormalMap;   // 1 = use normal map
    float  useSpecularMap; // 1 = use specular map
    float2 padding;        // keep 16-byte alignment
};

// Material
cbuffer MaterialCB : register(b1)
{
    Material material;
};

// Lights
cbuffer LightCB : register(b2)
{
    float4 eyePosition;         // camera position
    float4 globalAmbient;       // global ambient term
    Light  lights[MAX_LIGHTS];  // all lights
    uint   numLights;
    float  useBlinnPhong;       // 1 = Blinn, 0 = Phong
    float  padding[2];
};

// Texture/Sampler
Texture2D TextureMap   : register(t0);
Texture2D NormalMap    : register(t1);
Texture2D SpecularMap  : register(t2);
SamplerState SamplerWrap : register(s0);

// struct PixelInputType {
//     float4 position    : SV_POSITION; // clip-space
//     float3 worldPos    : WORLDPOS;    // world-space pos
//     float2 uv          : TEXCOORD;    // texture coords
//     float3x3 tbn       : TBN;         // tangent basis (from VS)
//     float3 worldNormal : NORMAL0;     // fallback if no normal map
// };

struct PixelInputType {
    float4 position    : SV_POSITION;
    float3 worldPos    : TEXCOORD0;
    float3 worldNormal : TEXCOORD1;
    float2 uv          : TEXCOORD2;
    float3 tbn0        : TEXCOORD3; // row 0 of TBN
    float3 tbn1        : TEXCOORD4; // row 1 of TBN
    float3 tbn2        : TEXCOORD5; // row 2 of TBN
};

float4 psmain(PixelInputType IN) : SV_TARGET
{
    float3 ambient, diffuse, specular;

    float3x3 TBN = float3x3(IN.tbn0, IN.tbn1, IN.tbn2);

    // Normal Mapping
    float3 N;
    if (material.useNormalMap > 0.5f)
    {
        // Sample and unpack normal map (tangent space)
        float3 normalTex = NormalMap.Sample(SamplerWrap, IN.uv).xyz * 2.0f - 1.0f;
        // Transform to world space using TBN
        N = normalize(mul(normalTex, TBN));
    }
    else
    {
        // Fallback: use world normal
        N = normalize(IN.worldNormal);
    }

    // Lighting
    computeLighting(
        lights,
        numLights,
        eyePosition.xyz,
        globalAmbient.xyz,
        (useBlinnPhong > 0.5f),
        material.specularPower,
        IN.worldPos,
        N,
        ambient,
        diffuse,
        specular
    );

    // Diffuse color
    float4 texColor = (material.useTexture > 0.5f) ? TextureMap.Sample(SamplerWrap, IN.uv) : float4(1.0f, 1.0f, 1.0f, 1.0f);

    // Specular color
    float3 specColor = material.specular.rgb;
    if (material.useSpecularMap > 0.5f)
        specColor = SpecularMap.Sample(SamplerWrap, IN.uv).rgb;

    // Final Lighting Combination
    float3 lit =    material.emissive.rgb     // emissive glow
                    + material.ambient.rgb      // material ambient
                    + ambient                   // global ambient
                    + diffuse                   // diffuse term
                    + (specular * specColor);   // specular term

    return float4(saturate(lit), 1.0f) * texColor;
}

