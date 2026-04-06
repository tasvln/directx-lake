#include "command_queue.h"

CommandQueue::CommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
    : device(device), type(type), fenceValue(0) 
{
    LOG_INFO(L"Initializing CommandQueue of type %d", type);

    D3D12_COMMAND_QUEUE_DESC desc{};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    throwFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue)));
    LOG_INFO(L"CommandQueue created");

    throwFailed(device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    LOG_INFO(L"Fence created with initial value %llu", fenceValue);

    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!fenceEvent) {
        LOG_ERROR(L"Failed to create fence event!");
        throw std::runtime_error("Failed to create fence event");
    }

    LOG_INFO(L"CommandQueue Initialized successfully");
}

CommandQueue::~CommandQueue() {
    flush();
    if (fenceEvent) {
        CloseHandle(fenceEvent);
        fenceEvent = nullptr;
    }
}

ComPtr<ID3D12CommandAllocator> CommandQueue::createCommandAllocator() {
    ComPtr<ID3D12CommandAllocator> allocator;
    throwFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&allocator)));
    LOG_INFO(L"Created new CommandAllocator");
    return allocator;
}

ComPtr<ID3D12GraphicsCommandList2> CommandQueue::createCommandList(ComPtr<ID3D12CommandAllocator> allocator) {
    ComPtr<ID3D12GraphicsCommandList2> list;
    throwFailed(device->CreateCommandList(0, type, allocator.Get(), nullptr, IID_PPV_ARGS(&list)));
    LOG_INFO(L"Created new CommandList (open)");
    return list;
}

ComPtr<ID3D12GraphicsCommandList2> CommandQueue::getCommandList() {
    LOG_INFO(L"Requesting CommandList");

    // Acquire allocator
    ComPtr<ID3D12CommandAllocator> alloc;
    if (!allocatorQueue.empty() && isFenceComplete(allocatorQueue.front().fenceValue)) {
        alloc = allocatorQueue.front().allocator;
        allocatorQueue.pop();
        throwFailed(alloc->Reset());
        LOG_INFO(L"Reusing CommandAllocator from pool");
    } else {
        alloc = createCommandAllocator();
    }

    // Acquire command list
    ComPtr<ID3D12GraphicsCommandList2> cmdList;
    if (!listQueue.empty()) {
        ListEntry entry = listQueue.front();
        listQueue.pop();
        cmdList = entry.list;
        throwFailed(cmdList->Reset(alloc.Get(), nullptr));
        LOG_INFO(L"Reusing CommandList from pool (reset)");
    } else {
        cmdList = createCommandList(alloc);
        LOG_INFO(L"Created new CommandList (open)");
    }

    liveListAllocMap[cmdList.Get()] = alloc;
    return cmdList;
}

UINT64 CommandQueue::executeCommandList(ComPtr<ID3D12GraphicsCommandList2> cmdList) {
    throwFailed(cmdList->Close());

    // Retrieve allocator
    auto it = liveListAllocMap.find(cmdList.Get());
    ComPtr<ID3D12CommandAllocator> allocatorForList = nullptr;
    if (it != liveListAllocMap.end()) {
        allocatorForList = it->second;
        liveListAllocMap.erase(it);
    } else {
        LOG_ERROR(L"Allocator not found for executed command list!");
    }

    // Execute
    ID3D12CommandList* lists[] = { cmdList.Get() };
    queue->ExecuteCommandLists(1, lists);

    // Signal fence
    UINT64 val = signalFence();
    allocatorQueue.push({ val, allocatorForList });

    // Return list to pool
    listQueue.push({ cmdList, allocatorForList });

    return val;
}

UINT64 CommandQueue::signalFence() {
    fenceValue++;
    throwFailed(queue->Signal(fence.Get(), fenceValue));
    return fenceValue;
}

bool CommandQueue::isFenceComplete(UINT64 value) {
    return fence->GetCompletedValue() >= value;
}

void CommandQueue::fenceWait(UINT64 value) {
    if (!isFenceComplete(value)) {
        throwFailed(fence->SetEventOnCompletion(value, fenceEvent));
        WaitForSingleObject(fenceEvent, DWORD_MAX);
    }
}

void CommandQueue::fenceFlush(UINT64 value) {
    fenceWait(value);
}

void CommandQueue::flush() {
    fenceWait(signalFence());
}
