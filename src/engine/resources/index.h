#pragma once

#include "utils/pch.h"

class IndexBuffer {
    public:
        IndexBuffer(
            ComPtr<ID3D12Device2> device, 
            const std::vector<uint32_t>& indices
        );
        ~IndexBuffer() = default;

        ComPtr<ID3D12Resource> getBuffer() const { 
            return buffer; 
        }

        UINT getSize() const { 
            return sizeInBytes; 
        }

        UINT getCount() const { 
            return count; 
        }

        D3D12_INDEX_BUFFER_VIEW getView() const {
            return bufferView;
        }

    private:
        D3D12_INDEX_BUFFER_VIEW bufferView{};
        ComPtr<ID3D12Resource> buffer;
        UINT sizeInBytes = 0;
        UINT count = 0;
};