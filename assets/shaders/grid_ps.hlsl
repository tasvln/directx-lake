struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float2 uv       : TEXCOORD1;
};

float4 psmain(PSInput input) : SV_TARGET
{
    // distance between grid lines
    float scale = 10.0f;

    // smooth grid lines
    float2 coord = input.worldPos.xz / scale;
    float2 grid = abs(frac(coord - 0.5f) - 0.5f) / fwidth(coord);
    float minLine = min(grid.x, grid.y);
    float gridLine = 1.0 - smoothstep(0.0, 1.0, minLine);

    // base and grid colors
    float3 baseColor = float3(0.05, 0.05, 0.05);
    float3 gridColor = float3(0.1, 0.1, 0.1);
    float3 finalColor = lerp(baseColor, gridColor, gridLine);

    // stronger center axes
    if (abs(input.worldPos.x) < 0.05f)
        finalColor = float3(1.0, 0.1, 0.1);   // X-axis
    if (abs(input.worldPos.z) < 0.05f)
        finalColor = float3(0.1, 0.4, 1.0);   // Z-axis

    return float4(finalColor, 1.0f);
}
