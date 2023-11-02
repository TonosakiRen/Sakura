#include "DirectionalLight.h"
#include <cassert>
#include "externals/DirectXTex/d3dx12.h"
#include "DirectXCommon.h"

using namespace DirectX;

void DirectionalLight::Initialize() {
    CreateConstBuffer();
    Map();
    UpdateDirectionalLight();
}

void DirectionalLight::CreateConstBuffer() {
    HRESULT result;
    ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
    // ヒーププロパティ
    CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    // リソース設定
    CD3DX12_RESOURCE_DESC resourceDesc =
        CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataDirectionalLight) + 0xff) & ~0xff);

    // 定数バッファの生成
    result = device->CreateCommittedResource(
        &heapProps, // アップロード可能
        D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(constBuff_.GetAddressOf()));
    assert(SUCCEEDED(result));
}

void DirectionalLight::Map() {
    // 定数バッファとのデータリンク
    HRESULT result = constBuff_->Map(0, nullptr, (void**)&constMap);
    assert(SUCCEEDED(result));
}

void DirectionalLight::UpdateDirectionalLight() {

    // 定数バッファに書き込み
    constMap->color = color_;
    constMap->direction = Normalize(direction_);
    constMap->intensity = intensity_;
}