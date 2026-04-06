#pragma once

#include "utils/pch.h"
#include "engine/resources/constant.h"

class Lighting {
public:
    Lighting(
        ComPtr<ID3D12Device2> device
    );

    void setLight(
        UINT index,
        LightType type,
        const XMFLOAT3& position,
        const XMFLOAT3& direction,
        float range,
        float innerAngle,
        float outerAngle,
        const XMFLOAT3& color,
        float intensity
    );

    // Update GPU buffer
    void updateGPU();

    // Get GPU handle
    ConstantBuffer* getCBV() { 
        return lightCBV.get(); 
    }

    void setEyePosition(const XMFLOAT3& eyePos) {
        lightData.eyePosition = XMFLOAT4(eyePos.x, eyePos.y, eyePos.z, 1.0f);
    }

    void setGlobalAmbient(const XMFLOAT3& ambient) {
        lightData.globalAmbient = XMFLOAT4(ambient.x, ambient.y, ambient.z, 1.0f);
    }

    void setBlinnPhong(bool enabled) {
        lightData.useBlinnPhong = enabled;
    }

private:
    LightBufferData lightData {};
    std::unique_ptr<ConstantBuffer> lightCBV;
};