#include "wave.h"

WaveSystem::WaveSystem(
    ComPtr<ID3D12Device2> device,
    CommandQueue*         commandQueue,
    DescriptorHeap*       srvHeap,
    uint32_t              resolution,
    float                 patchSize,
    float                 windSpeed,
    float                 windDirX,
    float                 windDirZ)
    : resolution(resolution), patchSize(patchSize)
{
    LOG_INFO(L"WaveSystem -> Initializing %ux%u grid, patch=%.0fm", 
             resolution, resolution, patchSize);

    // Fill constants struct — written to GPU constantBuffer each frame
    constants.resolution = resolution;
    constants.patchSize  = patchSize;
    constants.windSpeed  = windSpeed;
    constants.windDirX   = windDirX;
    constants.windDirZ   = windDirZ;
    constants.time       = 0.0f;

    // ConstantBuffer uses UPLOAD heap + persistent map — your existing class
    constantBuffer = std::make_unique<ConstantBuffer>(device, sizeof(WaveConstants));
    constantBuffer->update(&constants, sizeof(WaveConstants));

    // createTextures(device, srvHeap);
    // createRootSignature(device);
    // createPipelines(device);
    // buildSpectrum(device, commandQueue); // one-time GPU dispatch at startup

    LOG_INFO(L"WaveSystem -> Ready");
}