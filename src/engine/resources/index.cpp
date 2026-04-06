#include "index.h"

IndexBuffer::IndexBuffer(
    ComPtr<ID3D12Device2> device, 
    const std::vector<uint32_t>& indices
) {
    count = static_cast<UINT>(indices.size());
    sizeInBytes = static_cast<UINT>(sizeof(uint32_t) * count);

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes);

    throwFailed(device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&buffer)
    ));

    void* pData;

    throwFailed(buffer->Map(0, nullptr, &pData));
    memcpy(pData, indices.data(), sizeInBytes);
    buffer->Unmap(0, nullptr);

    bufferView.BufferLocation = buffer->GetGPUVirtualAddress();
    bufferView.Format = DXGI_FORMAT_R32_UINT;
    bufferView.SizeInBytes = sizeInBytes;

    LOG_INFO(L" -> Index buffer created with %d indices", count);
}

