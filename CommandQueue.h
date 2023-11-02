#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

class CommandQueue {
public:
    ~CommandQueue();

    void Create();
    void Execute(ID3D12GraphicsCommandList* commandList);
    void Signal();
    void WaitForGPU();

    operator ID3D12CommandQueue* () const { return commandQueue_.Get(); }

private:
    void Destroy();

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    HANDLE fenceEvent_;
    uint64_t fenceValue_;
};
