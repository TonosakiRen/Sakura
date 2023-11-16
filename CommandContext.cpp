#include "CommandContext.h"

#include "DirectXCommon.h"
#include "DescriptorHeap.h"

void CommandContext::Create() {
    HRESULT result = S_FALSE;

    auto device = DirectXCommon::GetInstance()->GetDevice();
    result = device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator_.ReleaseAndGetAddressOf()));

    assert(SUCCEEDED(result));

    result = device->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(),
        nullptr, IID_PPV_ARGS(commandList_.ReleaseAndGetAddressOf()));

    assert(SUCCEEDED(result));
}

void CommandContext::SetDescriptorHeap()
{
    auto graphics = DirectXCommon::GetInstance();
    ID3D12DescriptorHeap* ppHeaps[] = {
        (ID3D12DescriptorHeap*)graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
    };
    commandList_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

void CommandContext::Close() {
    HRESULT result = S_FALSE;
    FlushResourceBarriers();
    result = commandList_->Close();
    assert(SUCCEEDED(result));
}

void CommandContext::Reset() {
    HRESULT result = S_FALSE;

    result = commandAllocator_->Reset();
    assert(SUCCEEDED(result));

    result = commandList_->Reset(commandAllocator_.Get(), nullptr);
    assert(SUCCEEDED(result));

    auto graphics = DirectXCommon::GetInstance();
    ID3D12DescriptorHeap* ppHeaps[] = {
        (ID3D12DescriptorHeap*)graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
    };
    commandList_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    rootSignature_ = nullptr;
    pipelineState_ = nullptr;
}
