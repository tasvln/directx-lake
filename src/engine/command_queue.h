#pragma once

#include "utils/pch.h"
#include <queue>
#include <unordered_map>

class CommandQueue {
public:
    CommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
    ~CommandQueue();

    // Command Allocator
    ComPtr<ID3D12CommandAllocator> createCommandAllocator();

    // Command List
    ComPtr<ID3D12GraphicsCommandList2> createCommandList(ComPtr<ID3D12CommandAllocator> allocator);
    ComPtr<ID3D12GraphicsCommandList2> getCommandList();
    UINT64 executeCommandList(ComPtr<ID3D12GraphicsCommandList2> commandList);

    // Fence
    UINT64 signalFence();
    void fenceWait(UINT64 value);
    void fenceFlush(UINT64 value);
    bool isFenceComplete(UINT64 value);
    void flush();

    // Getters
    ComPtr<ID3D12CommandQueue> getCommandQueue() const { return queue; }
    ComPtr<ID3D12Fence> getFence() const { return fence; }
    UINT64 getFenceValue() const { return fenceValue; }
    HANDLE getFenceHandle() const { return fenceEvent; }

private:
    struct AllocatorEntry {
        UINT64 fenceValue = 0;
        ComPtr<ID3D12CommandAllocator> allocator;
    };

    struct ListEntry {
        ComPtr<ID3D12GraphicsCommandList2> list;
        ComPtr<ID3D12CommandAllocator> allocator;
    };

    ComPtr<ID3D12Device2> device;
    ComPtr<ID3D12CommandQueue> queue;

    D3D12_COMMAND_LIST_TYPE type;

    std::queue<AllocatorEntry> allocatorQueue;
    std::queue<ListEntry> listQueue;
    std::unordered_map<ID3D12GraphicsCommandList2*, ComPtr<ID3D12CommandAllocator>> liveListAllocMap;

    ComPtr<ID3D12Fence> fence;
    UINT64 fenceValue = 0;
    HANDLE fenceEvent = nullptr;
};
