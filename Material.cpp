#include "Material.h"
#include <cassert>
#include "externals/DirectXTex/d3dx12.h"
#include "DirectXCommon.h"

using namespace DirectX;

void Material::Initialize() {
    CreateConstBuffer();
    uvTransform_ = MakeIdentity4x4();
    Map();
    UpdateMaterial();
}

void Material::CreateConstBuffer() {
    HRESULT result;
    ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
    // ヒーププロパティ
    CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    // リソース設定
    CD3DX12_RESOURCE_DESC resourceDesc =
        CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataMaterial) + 0xff) & ~0xff);

    // 定数バッファの生成
    result = device->CreateCommittedResource(
        &heapProps, 
        D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(constBuff_.GetAddressOf()));
    assert(SUCCEEDED(result));
}

void Material::Map() {
    HRESULT result = constBuff_->Map(0, nullptr, (void**)&constMap);
    assert(SUCCEEDED(result));
}

void Material::UpdateMaterial() {
    Matrix4x4 matScale;
    Matrix4x4 matRot;
    Matrix4x4 matTrans;
    // スケール、回転、平行移動行列の計算
    matScale = MakeScaleMatrix(scale_);
    matRot = MakeIdentity4x4();
    matRot *= MakeRotateZMatrix(rotation_.z);
    matRot *= MakeRotateXMatrix(rotation_.x);
    matRot *= MakeRotateYMatrix(rotation_.y);
    matTrans = MakeTranslateMatrix(translation_);

    // ワールド行列の合成
    uvTransform_ = MakeIdentity4x4();
    uvTransform_ *= matScale;
    uvTransform_ *= matRot;
    uvTransform_ *= matTrans;

    constMap->uvTransform = uvTransform_;
    constMap->color = color_;
    constMap->enableLighting = enableLighting_;
}