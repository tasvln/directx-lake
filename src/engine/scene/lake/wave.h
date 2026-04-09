#pragma once

#include "utils/pch.h"

class Pipeline;
class ConstantBuffer;
class CommandQueue;
class DescriptorHeap;

struct WaveConstants { 
    float time; 
    float patchSize; 
    float windSpeed; 
    float windDirX; 
    float windDirZ; 
    uint32_t resolution; 
    XMFLOAT2 pad0;
};

static constexpr UINT WAVE_HEAP_START = 32;

class WaveSystem {
    public:
        WaveSystem(
            ComPtr<ID3D12Device2> device,
            CommandQueue* commandQueue,
            DescriptorHeap* srvHeap,
            uint32_t resolution = 512,
            float patchSize = 500.0f,
            float windSpeed = 8.0f,
            float windDirX = 1.0f,
            float windDirZ = 0.3f
        );
        ~WaveSystem() = default;

        // 
        void dispatch(ComPtr<ID3D12GraphicsCommandList> cmd, float time); 

        // Weather system can push new wind values live
        void setWind(float speed, float dirX, float dirZ);

        D3D12_GPU_DESCRIPTOR_HANDLE getHeightMapSRV() const { 
            return heightSRV; 
        }; 

        D3D12_GPU_DESCRIPTOR_HANDLE getNormalMapSRV() const { 
            return normalSRV; 
        };

    private:
        void createTextures(ComPtr<ID3D12Device2> device);
        void createDescriptors(ComPtr<ID3D12Device2> device, DescriptorHeap* heap);
        void createPipelines(ComPtr<ID3D12Device2> device);
        void buildInitialSpectrum(ComPtr<ID3D12Device2> device, CommandQueue* queue);

    private:
        // --- GPU textures ---
        // spectrumTex     : base Phillips spectrum, generated once at init
        // animatedTex     : spectrum with phases rotated by time (UAV, written Pass 1)
        // pingPong[2]     : intermediate FFT buffers (UAV, ping-pong between H and V pass)
        // heightTex       : final wave heights (UAV written by FFT, then SRV for water)
        // normalTex       : surface normals  (UAV written by Pass 3, then SRV for water)
        
        ComPtr<ID3D12Resource> spectrumTex;
        ComPtr<ID3D12Resource> animatedTex;
        ComPtr<ID3D12Resource> pingPong[2];
        ComPtr<ID3D12Resource> heightTex;
        ComPtr<ID3D12Resource> normalTex;

        D3D12_GPU_DESCRIPTOR_HANDLE spectrumSRV {};
        D3D12_GPU_DESCRIPTOR_HANDLE animatedUAV {};

        D3D12_GPU_DESCRIPTOR_HANDLE pingPongUAV[2] {};

        D3D12_GPU_DESCRIPTOR_HANDLE heightUAV {};
        D3D12_GPU_DESCRIPTOR_HANDLE heightSRV {};

        D3D12_GPU_DESCRIPTOR_HANDLE normalUAV {};
        D3D12_GPU_DESCRIPTOR_HANDLE normalSRV {};

        std::unique_ptr<Pipeline> spectrumPipeline; // phase update
        std::unique_ptr<Pipeline> fftPipeline;      // FFT pass
        std::unique_ptr<Pipeline> normalPipeline;   // normal generation
        
        std::unique_ptr<ConstantBuffer> constantBuffer;
        WaveConstants constants{};

        uint32_t resolution; 
        float patchSize;

        ComPtr<ID3D12Resource> m_spectrumBuffer;
};
