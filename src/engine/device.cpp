#include "device.h"

Device::Device(bool useWarp)
{

    enableDebugLayer();
    
    supportTearing = checkForTearingSupport();

    adapter = selectAdapter(useWarp);

    device = createDevice(adapter);

    LOG_INFO(L"Device->DirectX 12 device initialized.");
}

void Device::enableDebugLayer()
{
#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
        LOG_INFO(L"D3D12 Debug Layer enabled.");
        std::cout << "D3D12 Debug Layer enabled.\n";
    }
    else
    {
        LOG_ERROR(L"Failed to enable D3D12 Debug Layer!");
        std::cerr << "WARNING: Direct3D Debug Device is not available\n";
    }

    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

    ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
    {
        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
        dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

        DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
        {
            80 // IDXGISwapChain::GetContainingOutput noisy warning
        };
        DXGI_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = _countof(hide);
        filter.DenyList.pIDList = hide;
        dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);

        std::cout << "DXGI InfoQueue filter configured.\n";
    }
#endif
}

ComPtr<IDXGIAdapter4> Device::selectAdapter(bool useWarp)
{
    ComPtr<IDXGIFactory4> dxgiFactory;
    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    ComPtr<IDXGIAdapter4> dxgiAdapter4;

    throwFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    if (useWarp)
    {
        throwFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
        throwFailed(dxgiAdapter1.As(&dxgiAdapter4));
    }
    else
    {
        ComPtr<IDXGIAdapter1> bestAdapter1;
        size_t maxDedicatedVideoMemory = 0;

        for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            DXGI_ADAPTER_DESC1 desc;
            dxgiAdapter1->GetDesc1(&desc);

            if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
                desc.DedicatedVideoMemory > maxDedicatedVideoMemory)
            {
                maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
                bestAdapter1 = dxgiAdapter1;
            }
        }

        if (bestAdapter1)
        {
            throwFailed(bestAdapter1.As(&dxgiAdapter4));
        }
    }

    if (!dxgiAdapter4)
    {
        LOG_ERROR(L"No compatible GPU adapter found.");
        throw std::runtime_error("No compatible GPU adapter found.");
    }

    return dxgiAdapter4;
}

ComPtr<ID3D12Device2> Device::createDevice(ComPtr<IDXGIAdapter4> deviceAdapter)
{
    ComPtr<ID3D12Device2> id3d12device;

    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0};

    for (auto level : featureLevels)
    {
        if (SUCCEEDED(D3D12CreateDevice(deviceAdapter.Get(), level, IID_PPV_ARGS(&id3d12device))))
        {
            featureLevel = level;
            LOG_INFO(L"Created D3D12 device with feature level 0x%x", level);
            break;
        }
    }

    if (!id3d12device)
    {
        LOG_ERROR(L"Failed to create D3D12 device.");
        throw std::runtime_error("Failed to create D3D12 device.");
    }

#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> d3dInfoQueue;
    if (SUCCEEDED(id3d12device.As(&d3dInfoQueue)))
    {
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, FALSE);
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, FALSE);

        D3D12_MESSAGE_ID hide[] = {
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
        };
        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = _countof(hide);
        filter.DenyList.pIDList = hide;
        d3dInfoQueue->AddStorageFilterEntries(&filter);
    }
#endif

    return id3d12device;
}

bool Device::checkForTearingSupport()
{
    BOOL allowTearing = FALSE;

    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allowTearing,
                sizeof(allowTearing))))
            {
                allowTearing = FALSE;
            }
        }
    }

    return allowTearing == TRUE;
}
