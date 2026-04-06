#include "swapchain.h"

Swapchain::Swapchain(
    HWND hwnd, 
    ComPtr<ID3D12Device2> device,
    ComPtr<ID3D12CommandQueue> commandQueue, 
    UINT width, UINT height,
    uint32_t bufferCount, 
    bool tearingSupport
) :
    device(device),
    bufferCount(bufferCount),
    tearingSupport(tearingSupport)
{
    swapchain = createSwapchain(
        hwnd,
        commandQueue,
        width,
        height,
        bufferCount,
        tearingSupport
    );

    rtvHeap = std::make_unique<DescriptorHeap>(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, bufferCount);
    dsvHeap = std::make_unique<DescriptorHeap>(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
    srvHeap = std::make_unique<DescriptorHeap>(
        device,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        10000,  // max textures/materials you plan to use
        true  // shader visible
    );

    createRTVs();
    createDepthBuffer(width, height);
    
    LOG_INFO(L"Swapchain->Created Swapchain");
}

ComPtr<IDXGISwapChain4> Swapchain::createSwapchain(
    HWND& hwnd, 
    ComPtr<ID3D12CommandQueue> commandQueue, 
    UINT width, UINT height,
    uint32_t bufferCount, 
    bool tearingSupport
) {
    LOG_INFO(L"Swapchain -> Creating DXGIFactory...");

    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
    throwFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 
        1, 0 
    };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = bufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    // It is recommended to always allow tearing if tearing support is available.
    swapChainDesc.Flags = tearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    LOG_INFO(L"Swapchain -> Creating SwapChainForHwnd...");
    ComPtr<IDXGISwapChain1> tempSwapChain;
    throwFailed(dxgiFactory4->CreateSwapChainForHwnd(
        commandQueue.Get(),
        hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &tempSwapChain
    ));

    LOG_INFO(L"Swapchain -> Disabling Alt+Enter fullscreen toggle...");
    throwFailed(dxgiFactory4->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

    ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    throwFailed(tempSwapChain.As(&dxgiSwapChain4));

    LOG_INFO(L"Swapchain -> Swapchain creation finished successfully.");
    return dxgiSwapChain4;
}


void Swapchain::createDepthBuffer(UINT width, UINT height)
{
    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);

    throwFailed(device->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&depthBuffer)
    ));
    
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    device->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, dsvHeap->getCPUHandle(0));
}

// For each back buffer of the swap chain, 
// a single RTV is used to describe the resource.
void Swapchain::createRTVs() {
    auto rtvSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
 
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->getHeap()->GetCPUDescriptorHandleForHeapStart());

    backBuffers.resize(bufferCount);
    for (UINT i = 0; i < bufferCount; ++i) {
        throwFailed(swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i])));
        device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, rtvSize);
    }
}

void Swapchain::resize(UINT width, UINT height)
{
    for (auto& bb : backBuffers) 
        bb.Reset();
    depthBuffer.Reset();

    // Resize swapchain buffers
    UINT swapchainFlags = tearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    throwFailed(swapchain->ResizeBuffers(
        bufferCount,
        width,
        height,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        swapchainFlags
    ));

    // Recreate RTVs + depth buffer
    createRTVs();
    createDepthBuffer(width, height);

    LOG_INFO(L"Swapchain -> Resize complete");
}
