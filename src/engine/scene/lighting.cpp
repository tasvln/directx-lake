#include "lighting.h"

Lighting::Lighting(
    ComPtr<ID3D12Device2> device
) {
    lightCBV = std::make_unique<ConstantBuffer>(
        device,
        static_cast<UINT>(sizeof(LightBufferData))
    );

    lightData.numLights = 0;
    lightData.globalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    lightData.eyePosition = XMFLOAT4(0, 0, -5, 1); // arbitrary until updated
    lightData.useBlinnPhong = true;

    LOG_INFO(L"Lighting -> Light System Created Successfully.");
}

void Lighting::setLight(
    UINT index,
    LightType type,
    const XMFLOAT3& position,
    const XMFLOAT3& direction,
    float range,
    float innerAngle,
    float outerAngle,
    const XMFLOAT3& color,
    float intensity
) {
    if (index >= MAX_LIGHTS) return;

    auto& light = lightData.lights[index];
    light.type = static_cast<int>(type);
    light.enabled = 1.0f;
    light.position = XMFLOAT4(position.x, position.y, position.z, 1.0f);
    light.direction = XMFLOAT4(direction.x, direction.y, direction.z, 0.0f);
    light.range = range;
    light.innerAngle = innerAngle;
    light.outerAngle = outerAngle;
    light.color = XMFLOAT4(color.x, color.y, color.z, 1.0f);
    light.intensity = intensity;

    if (index >= lightData.numLights) {
        lightData.numLights = index + 1;
    }
}

void Lighting::updateGPU() {
    lightCBV->update(&lightData, sizeof(LightBufferData));
    LOG_INFO(L"Lighting -> Updated GPU Successfully.");
}

