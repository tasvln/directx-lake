#pragma once

#include "utils/pch.h"
#include "descriptor_heap.h"

class Swapchain {
    public:
        Swapchain(
            HWND hwnd, 
            ComPtr<ID3D12Device2> device,
            ComPtr<ID3D12CommandQueue> commandQueue, 
            UINT width, UINT height, 
            uint32_t bufferCount, 
            bool tearingSupport
        );

        ~Swapchain() = default;

        ComPtr<IDXGISwapChain4> createSwapchain(
            HWND& hwnd, 
            ComPtr<ID3D12CommandQueue> commandQueue, 
            uint32_t width, 
            uint32_t height, 
            uint32_t bufferCount, 
            bool tearingSupport
        );

        void resize(UINT width, UINT height);

        ComPtr<IDXGISwapChain4> getSwapchain() const {
            return swapchain;
        }

        ComPtr<ID3D12Resource> getBackBuffer(size_t index) const {
            return backBuffers[index];
        }

        ComPtr<ID3D12Resource> getDepthBuffer() const { 
            return depthBuffer; 
        }

        DescriptorHeap* getRTVHeap() const {
            return rtvHeap.get();
        }
        
        DescriptorHeap* getDSVHeap() const {
            return dsvHeap.get();
        }
        
        DescriptorHeap* getSRVHeap() const {
            return srvHeap.get();
        }

    private:
        void createRTVs(); // render target views -> yes that's what it means :)
        void createDepthBuffer(UINT width, UINT height);

    private:
        ComPtr<IDXGISwapChain4> swapchain;
        ComPtr<ID3D12Device2> device;
        UINT bufferCount;
        bool tearingSupport;

        std::vector<ComPtr<ID3D12Resource>> backBuffers;
        ComPtr<ID3D12Resource> depthBuffer;

        std::unique_ptr<DescriptorHeap> rtvHeap;
        std::unique_ptr<DescriptorHeap> dsvHeap;

        std::unique_ptr<DescriptorHeap> srvHeap;
};