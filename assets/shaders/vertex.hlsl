struct VertexInput {
    float4 position : POSITION;
    float3 normal   : NORMAL0;
    float4 tangent  : TANGENT;
    float2 uv       : TEXCOORD0;
};

struct VertexOutput {
    float4 position    : SV_POSITION;
    float3 worldPos    : TEXCOORD0;
    float3 worldNormal : TEXCOORD1;
    float2 uv          : TEXCOORD2;
    float3 tbn0        : TEXCOORD3; // row 0 of TBN
    float3 tbn1        : TEXCOORD4; // row 1 of TBN
    float3 tbn2        : TEXCOORD5; // row 2 of TBN
};

// Constant buffer for MVP
cbuffer ModelViewProjectionCB : register(b0)
{
    matrix model;
    matrix viewProj;
};

float3x3 inverse3x3(float3x3 m)
{
    float3 a = m[0];
    float3 b = m[1];
    float3 c = m[2];

    float3 r0 = cross(b, c);
    float3 r1 = cross(c, a);
    float3 r2 = cross(a, b);

    float invDet = 1.0 / dot(r2, c);
    return float3x3(r0 * invDet, r1 * invDet, r2 * invDet);
}

VertexOutput vsmain(VertexInput input)
{
    VertexOutput output;

    float4 worldPosition = mul(input.position, model);
    output.position = mul(worldPosition, viewProj);
    output.worldPos = worldPosition.xyz;
    output.uv = input.uv;

    float3x3 normalMatrix = transpose(inverse3x3((float3x3)model));

    float3 worldNormal    = normalize(mul(input.normal, normalMatrix));
    float3 worldTangent   = normalize(mul(input.tangent.xyz, (float3x3)model));

    // Orthonormalize
    worldTangent = normalize(worldTangent - worldNormal * dot(worldNormal, worldTangent));

    float3 worldBitangent = cross(worldNormal, worldTangent) * input.tangent.w;

    output.worldNormal = worldNormal;

    output.tbn0 = worldTangent;
    output.tbn1 = worldBitangent;
    output.tbn2 = worldNormal;

    return output;
}

