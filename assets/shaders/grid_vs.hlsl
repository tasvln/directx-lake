cbuffer MVPConstant : register(b0)
{
    matrix model;
    matrix viewProj;
};

struct VSInput
{
    float4 position : POSITION;
    float3 normal   : NORMAL0;
    float4 tangent  : TANGENT;
    float2 uv       : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float2 uv       : TEXCOORD1;
};

VSOutput vsmain(VSInput input)
{
    VSOutput output;

    // Transform position to world space
    float4 worldPos = mul(input.position, model);
    output.worldPos = worldPos.xyz;

    // Apply ViewProjection for clip-space position
    output.position = mul(worldPos, viewProj);

    output.uv = input.uv;
    return output;
}
