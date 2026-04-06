#pragma once

#include "utils/pch.h"

class ConstantBuffer {
    public:
        ConstantBuffer(
            ComPtr<ID3D12Device2> device, 
            UINT size
        );
        ~ConstantBuffer();

        ComPtr<ID3D12Resource> getBuffer() const { 
            return buffer; 
        }

        UINT getSize() const { 
            return sizeInBytes; 
        }

        void update(const void* data, size_t size);

        D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() const { 
            return buffer->GetGPUVirtualAddress(); 
        }

    private:
        ComPtr<ID3D12Resource> buffer;
        UINT sizeInBytes = 0;
        UINT8* mappedData = nullptr;
};