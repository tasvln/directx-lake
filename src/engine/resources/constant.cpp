#include "constant.h"

ConstantBuffer::ConstantBuffer(
    ComPtr<ID3D12Device2> device, 
    UINT size
) :
    sizeInBytes((size + 255) & ~255)
{
    LOG_INFO(L"ConstantBuffer -> Creating constant buffer of size %d bytes", sizeInBytes);

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

    CD3DX12_RANGE readRange(0, 0);
    throwFailed(buffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
    
    LOG_INFO(L"ConstantBuffer -> Buffer mapped for CPU writes");
}

ConstantBuffer::~ConstantBuffer() {
    if (buffer) 
        buffer->Unmap(0, nullptr);
}

void ConstantBuffer::update(const void* data, size_t size) {
    if (size > sizeInBytes) {
        LOG_ERROR(L"ConstantBuffer -> Update size %d exceeds buffer size %d", size, sizeInBytes);
        return;
    }
    memcpy(mappedData, data, size);
}