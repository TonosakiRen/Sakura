#include "CommandQueue.h"

#include <cassert>

#include "DirectXCommon.h"

CommandQueue::~CommandQueue() {
    Destroy();
}

void CommandQueue::Create() {
    HRESULT result;

    auto device = DirectXCommon::GetInstance()->GetDevice();

    D3D12_COMMAND_QUEUE_DESC desc{};
    result = device->CreateCommandQueue(&desc, IID_PPV_ARGS(commandQueue_.ReleaseAndGetAddressOf()));

    assert(SUCCEEDED(result));

    result = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.ReleaseAndGetAddressOf()));

    if (fenceEvent_) {
        fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        assert(fenceEvent_);
    }

    fenceValue_ = 0;
}

void CommandQueue::Execute(ID3D12GraphicsCommandList* commandList) {
    ID3D12CommandList* ppCmdList[] = {commandList};
    commandQueue_->ExecuteCommandLists(_countof(ppCmdList), ppCmdList);
}

void CommandQueue::Signal() {
    HRESULT result;
    result = commandQueue_->Signal(fence_.Get(), ++fenceValue_);
    assert(SUCCEEDED(result));
}

void CommandQueue::WaitForGPU() {
    HRESULT result;

    if (fence_->GetCompletedValue() < fenceValue_) {
        result = fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
        WaitForSingleObject(fenceEvent_, INFINITE);
    }
}

void CommandQueue::Destroy() {
    WaitForGPU();
    if (fenceEvent_) {
        CloseHandle(fenceEvent_);
        fenceEvent_ = nullptr;
    }
}
