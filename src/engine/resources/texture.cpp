
#include "texture.h"
#include "engine/descriptor_heap.h"

Texture::Texture(
    ComPtr<ID3D12Device2> device,
    ComPtr<ID3D12GraphicsCommandList> cmdList,
    DescriptorHeap* srvHeap,
    const std::wstring& path,
    UINT descriptorIndex
) {
    LOG_INFO(L"[Texture] Descriptor index used: %u", descriptorIndex);
    loadFromFile(device, cmdList, srvHeap, path, descriptorIndex);
    LOG_INFO(L"[Texture] -> after loadFromFile Function ->  Descriptor index used: %u", descriptorIndex);

}

void Texture::loadFromFile(
    ComPtr<ID3D12Device2> device,
    ComPtr<ID3D12GraphicsCommandList> cmdList,
    DescriptorHeap* srvHeap,
    const std::wstring& path,
    UINT descriptorIndex
) {
    LOG_INFO(L"Texture -> Loading texture from: %s", path.c_str());

    // Load an image file from disk
    ScratchImage image;
    HRESULT hr = LoadFromWICFile(
        path.c_str(),
        WIC_FLAGS_FORCE_SRGB,
        nullptr,
        image
    );
    
    if (FAILED(hr)) throw std::runtime_error("Failed to load image with DirectXTex.");

    const TexMetadata& meta = image.GetMetadata();
    const Image* img = image.GetImage(0, 0, 0);

    // Describe the texture resource
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = static_cast<UINT>(meta.width);
    texDesc.Height = static_cast<UINT>(meta.height);
    texDesc.DepthOrArraySize = static_cast<UINT16>(meta.arraySize);
    texDesc.MipLevels = static_cast<UINT16>(meta.mipLevels);
    texDesc.Format = meta.format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Create GPU texture
    CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
    hr = device->CreateCommittedResource(
        &defaultHeap,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&resource)
    );
    if (FAILED(hr)) throw std::runtime_error("Failed to create texture resource.");

    // Create upload buffer
    UINT64 uploadBufferSize = GetRequiredIntermediateSize(
        resource.Get(), 
        0, 
        static_cast<UINT>(meta.mipLevels)
    );

    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

    hr = device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &uploadBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadHeap)
    );
    if (FAILED(hr)) throw std::runtime_error("Failed to create texture upload heap.");

    // Prepare subresources
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    PrepareUpload(
        device.Get(), 
        image.GetImages(), 
        image.GetImageCount(), 
        meta, 
        subresources
    );

    // Upload
    UpdateSubresources(
        cmdList.Get(), 
        resource.Get(), 
        uploadHeap.Get(), 
        0, 
        0, 
        static_cast<UINT>(subresources.size()), 
        subresources.data()
    );

    // Transition to PIXEL_SHADER_RESOURCE
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    cmdList->ResourceBarrier(1, &barrier);

    // Create SRV in descriptor heap
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = meta.format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = static_cast<UINT>(meta.mipLevels);

    auto cpuHandle = srvHeap->getCPUHandle(descriptorIndex);

    device->CreateShaderResourceView(resource.Get(), &srvDesc, cpuHandle);

    gpuHandle = srvHeap->getGPUHandle(descriptorIndex);
    LOG_INFO(L"[Texture] GPU handle after SRV creation = 0x%llX", gpuHandle.ptr);

    LOG_INFO(L"Texture -> Successfully loaded %s", path.c_str());
}
