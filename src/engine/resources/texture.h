#pragma once

#include "utils/pch.h"

class DescriptorHeap;

class Texture {
    public:
        Texture(
            ComPtr<ID3D12Device2> device,
            ComPtr<ID3D12GraphicsCommandList> cmdList,
            DescriptorHeap* srvHeap,
            const std::wstring& path,
            UINT descriptorIndex
        );

        ~Texture() = default;

        void loadFromFile(
            ComPtr<ID3D12Device2> device,
            ComPtr<ID3D12GraphicsCommandList> cmdList,
            DescriptorHeap* srvHeap,
            const std::wstring& path,
            UINT descriptorIndex
        );

        ComPtr<ID3D12Resource> getResource() const { 
            return resource; 
        }

        D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle() const { 
            return gpuHandle; 
        }

    private:
        ComPtr<ID3D12Resource> resource;
        ComPtr<ID3D12Resource> uploadHeap;

        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;

        void loadFromFile(ComPtr<ID3D12Device2> device, const std::wstring& path);
};