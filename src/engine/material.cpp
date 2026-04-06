#pragma once

#include "material.h"
#include "engine/resources/texture.h"

void Material::bind(ID3D12GraphicsCommandList* cmdList, UINT rootIndex) {
    LOG_INFO(L"[Material] bind() called");

    if (!texture) {
        LOG_INFO(L"[Material] texture is nullptr!");
        return;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = texture->getGPUHandle();

    if (gpuHandle.ptr == 0) {
        LOG_ERROR(L"[Material] GPU handle is null! Texture may not be uploaded yet.");
        return;
    }

    LOG_INFO(L"[Material] GPU handle = 0x%llX", gpuHandle.ptr);

    cmdList->SetGraphicsRootDescriptorTable(rootIndex, gpuHandle);
    LOG_INFO(L"[Material] Texture bound successfully: rootIndex=%u, GPU handle=0x%llX",
             rootIndex, gpuHandle.ptr);
}


