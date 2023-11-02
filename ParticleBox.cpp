#include "ParticleBox.h"
#include "externals/DirectXTex/DirectXTex.h"
#include <d3dcompiler.h>
#include "DirectXCommon.h"

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

DirectXCommon* ParticleBox::sDirectXCommon = nullptr;
UINT ParticleBox::sDescriptorHandleIncrementSize = 0;
ID3D12GraphicsCommandList* ParticleBox::sCommandList = nullptr;
std::unique_ptr<RootSignature> ParticleBox::sRootSignature;
std::unique_ptr<PipelineState> ParticleBox::sPipelineState;

ParticleBox::ParticleBox(uint32_t particleNum) : kParticleBoxNum(particleNum) {
}

void ParticleBox::StaticInitialize() {
    sDirectXCommon = DirectXCommon::GetInstance();
    InitializeGraphicsPipeline();
}

void ParticleBox::PreDraw(ID3D12GraphicsCommandList* commandList) {
    assert(ParticleBox::sCommandList == nullptr);

    sCommandList = commandList;

    commandList->SetPipelineState(*sPipelineState);
    commandList->SetGraphicsRootSignature(*sRootSignature);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ParticleBox::PostDraw() {
    sCommandList = nullptr;
}

ParticleBox* ParticleBox::Create(uint32_t particleNum) {
    ParticleBox* object3d = new ParticleBox(particleNum);
    assert(object3d);

    object3d->Initialize();

    return object3d;
}

void ParticleBox::InitializeGraphicsPipeline() {
    HRESULT result = S_FALSE;
    ComPtr<IDxcBlob> vsBlob;
    ComPtr<IDxcBlob> psBlob;
    ComPtr<ID3DBlob> errorBlob;

    {

        CD3DX12_DESCRIPTOR_RANGE descRangeSRV;
        descRangeSRV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0

        CD3DX12_DESCRIPTOR_RANGE descRangeSRVTexture;
        descRangeSRVTexture.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); // t1

        CD3DX12_ROOT_PARAMETER rootparams[int(RootParameter::parameterNum)] = {};
        rootparams[int(RootParameter::kWorldTransform)].InitAsDescriptorTable(1, &descRangeSRV, D3D12_SHADER_VISIBILITY_ALL);
        rootparams[int(RootParameter::kViewProjection)].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
        rootparams[int(RootParameter::kTexture)].InitAsDescriptorTable(1, &descRangeSRVTexture, D3D12_SHADER_VISIBILITY_ALL);
        rootparams[int(RootParameter::kMaterial)].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
        rootparams[int(RootParameter::kDirectionalLight)].InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_ALL);


        // スタティックサンプラー
        CD3DX12_STATIC_SAMPLER_DESC samplerDesc =
            CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

        // ルートシグネチャの設定
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.pParameters = rootparams;
        rootSignatureDesc.NumParameters = _countof(rootparams);
        rootSignatureDesc.pStaticSamplers = &samplerDesc;
        rootSignatureDesc.NumStaticSamplers = 1;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        sRootSignature->Create(rootSignatureDesc);

    }

}

void ParticleBox::CreateMesh() {
    HRESULT result = S_FALSE;

    vertices_ = {
        //  x      y      z       nx     ny    nz       u     v
        // 前
          {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}}, // 左下
          {{-1.0f, +1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}}, // 左上
          {{+1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}}, // 右下
          {{+1.0f, +1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}}, // 右上
          // 後
          {{+1.0f, -1.0f, +1.0f}, {0.0f, 0.0f, +1.0f}, {0.0f, 1.0f}}, // 左下
          {{+1.0f, +1.0f, +1.0f}, {0.0f, 0.0f, +1.0f}, {0.0f, 0.0f}}, // 左上
          {{-1.0f, -1.0f, +1.0f}, {0.0f, 0.0f, +1.0f}, {1.0f, 1.0f}}, // 右下
          {{-1.0f, +1.0f, +1.0f}, {0.0f, 0.0f, +1.0f}, {1.0f, 0.0f}}, // 右上
          // 左
          {{-1.0f, -1.0f, +1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // 左下
          {{-1.0f, +1.0f, +1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 左上
          {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // 右下
          {{-1.0f, +1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // 右上
          // 右
          {{+1.0f, -1.0f, -1.0f}, {+1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // 左下
          {{+1.0f, +1.0f, -1.0f}, {+1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 左上
          {{+1.0f, -1.0f, +1.0f}, {+1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // 右下
          {{+1.0f, +1.0f, +1.0f}, {+1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // 右上
          // 下
          {{+1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, // 左下
          {{+1.0f, -1.0f, +1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // 左上
          {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}, // 右下
          {{-1.0f, -1.0f, +1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // 右上
          // 上
          {{-1.0f, +1.0f, -1.0f}, {0.0f, +1.0f, 0.0f}, {0.0f, 1.0f}}, // 左下
          {{-1.0f, +1.0f, +1.0f}, {0.0f, +1.0f, 0.0f}, {0.0f, 0.0f}}, // 左上
          {{+1.0f, +1.0f, -1.0f}, {0.0f, +1.0f, 0.0f}, {1.0f, 1.0f}}, // 右下
          {{+1.0f, +1.0f, +1.0f}, {0.0f, +1.0f, 0.0f}, {1.0f, 0.0f}}, // 右上
    };

    // 頂点インデックス
    indices_ = { 0,  1,  3,  3,  2,  0,

                4,  5,  7,  7,  6,  4,

                8,  9,  11, 11, 10, 8,

                12, 13, 15, 15, 14, 12,

                16, 17, 19, 19, 18, 16,

                20, 21, 23, 23, 22, 20
    };

    // 頂点データのサイズ
    UINT sizeVB = static_cast<UINT>(sizeof(VertexData) * vertices_.size());

    {
        // ヒーププロパティ
        CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        // リソース設定
        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeVB);

        // 頂点バッファ生成
        result = sDirectXCommon->GetDevice()->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&vertBuff_));
        assert(SUCCEEDED(result));
    }

    // インデックスデータのサイズ
    UINT sizeIB = static_cast<UINT>(sizeof(uint16_t) * indices_.size());

    {
        // ヒーププロパティ
        CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        // リソース設定
        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeIB);

        // インデックスバッファ生成
        result = sDirectXCommon->GetDevice()->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&indexBuff_));
        assert(SUCCEEDED(result));
    }

    // インスタンシングデータのサイズ
    UINT sizeINB = static_cast<UINT>(sizeof(InstancingBufferData) * kParticleBoxNum);

    {
        // ヒーププロパティ
        CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        // リソース設定
        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeINB);

        // インスタンシングバッファ生成
        result = sDirectXCommon->GetDevice()->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&instancingBuff_));
        assert(SUCCEEDED(result));
    }

    VertexData* vertMap = nullptr;
    result = vertBuff_->Map(0, nullptr, reinterpret_cast<void**>(&vertMap));
    if (SUCCEEDED(result)) {
        std::copy(vertices_.begin(), vertices_.end(), vertMap);
        vertBuff_->Unmap(0, nullptr);
    }

    uint16_t* indexMap = nullptr;
    result = indexBuff_->Map(0, nullptr, reinterpret_cast<void**>(&indexMap));
    if (SUCCEEDED(result)) {
        std::copy(indices_.begin(), indices_.end(), indexMap);
        indexBuff_->Unmap(0, nullptr);
    }

    result = instancingBuff_->Map(0, nullptr, reinterpret_cast<void**>(&instanceMap));
    if (SUCCEEDED(result)) {
        particleDatas_.resize(sizeof(InstancingBufferData) * kParticleBoxNum);
        for (uint32_t index = 0; index < kParticleBoxNum; ++index) {
            particleDatas_[index].matWorld = MakeIdentity4x4();
            instanceMap[index].matWorld = particleDatas_[index].matWorld;
        }
    }

    vbView_.BufferLocation = vertBuff_->GetGPUVirtualAddress();
    vbView_.SizeInBytes = sizeVB;
    vbView_.StrideInBytes = sizeof(vertices_[0]);

    ibView_.BufferLocation = indexBuff_->GetGPUVirtualAddress();
    ibView_.Format = DXGI_FORMAT_R16_UINT;
    ibView_.SizeInBytes = sizeIB;

    D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
    instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    instancingSrvDesc.Buffer.FirstElement = 0;
    instancingSrvDesc.Buffer.NumElements = kParticleBoxNum;
    instancingSrvDesc.Buffer.StructureByteStride = sizeof(InstancingBufferData);
    UINT incrementSize = DirectXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    srvHandle_ = DirectXCommon::GetInstance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(instancingBuff_.Get(), &instancingSrvDesc, srvHandle_);
}

void ParticleBox::Initialize() {
    assert(sDirectXCommon->GetDevice());
    material_.Initialize();
    CreateMesh();
}

void ParticleBox::Draw(const std::vector<InstancingBufferData>& bufferData, const ViewProjection& viewProjection, const DirectionalLight& directionalLight, const Vector4& color, const uint32_t textureHadle) {
    assert(sDirectXCommon->GetDevice());
    assert(sCommandList);
    assert(!bufferData.empty());

    //マッピング
    memcpy(instanceMap, bufferData.data(), sizeof(bufferData[0]) * bufferData.size());

    material_.color_ = color;
    material_.UpdateMaterial();

    sCommandList->IASetVertexBuffers(0, 1, &vbView_);
    sCommandList->IASetIndexBuffer(&ibView_);
    sCommandList->SetGraphicsRootDescriptorTable(0, srvHandle_);
    sCommandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootParameter::kViewProjection), viewProjection.GetGPUVirtualAddress());
    sCommandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootParameter::kMaterial), material_.GetGPUVirtualAddress());
    sCommandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootParameter::kDirectionalLight), directionalLight.GetGPUVirtualAddress());

    TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(sCommandList, static_cast<UINT>(RootParameter::kTexture), textureHadle);

    sCommandList->DrawIndexedInstanced(static_cast<UINT>(indices_.size()), static_cast<UINT>(bufferData.size()), 0, 0, 0);
}
