#ifndef LIGHTING_HLSL
#define LIGHTING_HLSL

#define MAX_LIGHTS 16

#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct Light
{
    float4 position;
    float4 direction;
    float4 color;
    float range;
    float innerAngle;
    float outerAngle;
    float intensity;
    int   type;
    float enabled; // use float instead of bool for alignment
    float2 pad;    // align to 16 bytes
};

// Diffuse
float3 doDiffuse(
    Light light, 
    float3 L, 
    float3 N
) {
    return light.color.rgb * (max(dot(N, L), 0.0f) * light.intensity);
}

// Specular
float3 doSpecular(
    Light light, 
    float3 V, 
    float3 L, 
    float3 N, 
    bool useBlinnPhong, 
    float specPower
) {
    float spec = 0.0f;

    if (useBlinnPhong)
    {
        float3 H = normalize(L + V);
        spec = pow(max(dot(N, H), 0.0f), specPower);
    }
    else
    {
        float3 R = reflect(-L, N);
        spec = pow(max(dot(R, V), 0.0f), specPower);
    }
    
    return light.color.rgb * (spec * light.intensity);
}

// Attenuation
float doAttenuation(
    Light light, 
    float distance
) {
    return saturate(1.0f - distance / light.range);
}

// Spotlight
float doSpotCone(
    Light light, 
    float3 L
) {
    float cosAngle = dot(normalize(light.direction.xyz), -L);
    float innerCos = cos(light.innerAngle);
    float outerCos = cos(light.outerAngle);
    return saturate((cosAngle - outerCos) / max(innerCos - outerCos, 0.001f));
}

// Apply single light
void applyLight(
    Light light,
    float3 V,
    float3 P,
    float3 N,
    inout float3 diffuseAcc,
    inout float3 specAcc,
    bool useBlinnPhong,
    float specPower
)
{
    if (light.enabled < 0.5f) 
        return; // treat as boolean

    float3 L;
    float attenuation = 1.0f;
    float spotFactor = 1.0f;

    if (light.type == DIRECTIONAL_LIGHT)
    {
        L = normalize(-light.direction.xyz);
    }
    else
    {
        L = light.position.xyz - P;
        float distance = length(L);
        L /= distance;
        attenuation = doAttenuation(light, distance);

        if (light.type == SPOT_LIGHT)
            spotFactor = doSpotCone(light, L);
    }

    diffuseAcc += doDiffuse(light,L,N) * attenuation * spotFactor;
    specAcc += doSpecular(light, V, L, N, useBlinnPhong, specPower) * attenuation * spotFactor;
}

// Compute lighting
void computeLighting(
    Light lights[MAX_LIGHTS],
    uint numLights,
    float3 eyePosition,
    float3 globalAmbient,
    bool useBlinnPhong,
    float specPower,
    float3 P,
    float3 N,
    out float3 ambient,
    out float3 diffuse,
    out float3 spec
) {
    float3 V = normalize(eyePosition - P);

    ambient = globalAmbient;
    diffuse = float3(0, 0, 0);
    spec = float3(0, 0, 0);

    // [unroll]
    for(uint i = 0; i < numLights; i++) {
        applyLight(
            lights[i],
            V,
            P,
            N,
            diffuse,
            spec,
            useBlinnPhong,
            specPower
        );
    }

    ambient = saturate(ambient);
    diffuse = saturate(diffuse);
    spec = saturate(spec);
}


#endif
