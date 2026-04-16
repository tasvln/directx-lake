#pragma once

#include "material.h"
#include "engine/resources/texture.h"

void Material::bind(ID3D12GraphicsCommandList* cmdList, UINT rootIndex) {
    if (!texture) {
        LOG_INFO(L"[Material] texture is nullptr!");
        return;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = texture->getGPUHandle();

    if (gpuHandle.ptr == 0) {
        LOG_ERROR(L"[Material] GPU handle is null! Texture may not be uploaded yet.");
        return;
    }

    cmdList->SetGraphicsRootDescriptorTable(rootIndex, gpuHandle);
}


