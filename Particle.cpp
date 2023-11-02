#include "Particle.h"
#include "externals/DirectXTex/DirectXTex.h"
#include <d3dcompiler.h>
#include "DirectXCommon.h"

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace Microsoft::WRL;

DirectXCommon* Particle::sDirectXCommon = nullptr;
UINT Particle::sDescriptorHandleIncrementSize = 0;
ID3D12GraphicsCommandList* Particle::sCommandList = nullptr;
std::unique_ptr<RootSignature> Particle::sRootSignature;
std::unique_ptr<PipelineState> Particle::sPipelineState;

void Particle::StaticInitialize() {
    sDirectXCommon = DirectXCommon::GetInstance();
    InitializeGraphicsPipeline();
}

void Particle::PreDraw(ID3D12GraphicsCommandList* commandList) {
    assert(Particle::sCommandList == nullptr);

    sCommandList = commandList;

    commandList->SetPipelineState(*sPipelineState);
    commandList->SetGraphicsRootSignature(*sRootSignature);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Particle::PostDraw() {
    sCommandList = nullptr;
}

Particle* Particle::Create() {
    Particle* object3d = new Particle();
    assert(object3d);

    object3d->Initialize();

    return object3d;
}

void Particle::InitializeGraphicsPipeline() {
    HRESULT result = S_FALSE;
    ComPtr<IDxcBlob> vsBlob;
    ComPtr<IDxcBlob> psBlob;
    ComPtr<ID3DBlob> errorBlob;

    vsBlob = sDirectXCommon->CompileShader(L"ParticleVS.hlsl", L"vs_6_0");
    assert(vsBlob != nullptr);

    psBlob = sDirectXCommon->CompileShader(L"ParticlePS.hlsl", L"ps_6_0");
    assert(psBlob != nullptr);

    sRootSignature = std::make_unique<RootSignature>();
    sPipelineState = std::make_unique<PipelineState>();

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

    {

        // 頂点レイアウト
        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
          {
           "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
           D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
          {
           "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT,
           D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };

        // グラフィックスパイプライン
        D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
        gpipeline.VS = CD3DX12_SHADER_BYTECODE(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize());
        gpipeline.PS = CD3DX12_SHADER_BYTECODE(psBlob->GetBufferPointer(), psBlob->GetBufferSize());


        gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

        gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

        gpipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);


        D3D12_RENDER_TARGET_BLEND_DESC blenddesc{};
        blenddesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        blenddesc.BlendEnable = true;
        blenddesc.BlendOp = D3D12_BLEND_OP_ADD;
        blenddesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blenddesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

        blenddesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blenddesc.SrcBlendAlpha = D3D12_BLEND_ONE;
        blenddesc.DestBlendAlpha = D3D12_BLEND_ZERO;

        gpipeline.BlendState.RenderTarget[0] = blenddesc;

        gpipeline.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

        gpipeline.InputLayout.pInputElementDescs = inputLayout;
        gpipeline.InputLayout.NumElements = _countof(inputLayout);

        gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        gpipeline.NumRenderTargets = 1;
        gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        gpipeline.SampleDesc.Count = 1;


        gpipeline.pRootSignature = *sRootSignature;

        // グラフィックスパイプラインの生成
        sPipelineState->Create(gpipeline);
    }
   
}

void Particle::CreateMesh() {
    HRESULT result = S_FALSE;

    vertices_.resize(4);

    //左下
    vertices_[0].pos = { -0.5f,-0.5f,0.0f };
    vertices_[0].uv = { 0.0f,1.0f };
	//左上
    vertices_[1].pos = { -0.5f,0.5f,0.0f};
    vertices_[1].uv = { 0.0f,0.0f };
	//右上
    vertices_[2].pos = { 0.5f,0.5f,0.0f};
    vertices_[2].uv = { 1.0f,0.0f };
	//右下
    vertices_[3].pos = { 0.5f,-0.5f,0.0f};
    vertices_[3].uv = { 1.0f,1.0f };

    // 頂点インデックスの設定
    indices_ = { 0,  1,  2, 0, 2, 3 };

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
    UINT sizeINB = static_cast<UINT>(sizeof(InstancingBufferData) * kParticleNum);

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
        particleDatas_.resize(sizeof(InstancingBufferData) * kParticleNum);
        for (uint32_t index = 0; index < kParticleNum; ++index) {
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
    instancingSrvDesc.Buffer.NumElements = kParticleNum;
    instancingSrvDesc.Buffer.StructureByteStride = sizeof(InstancingBufferData);
    UINT incrementSize = DirectXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    srvHandle_ = DirectXCommon::GetInstance()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(instancingBuff_.Get(), &instancingSrvDesc, srvHandle_);
}

void Particle::Initialize() {
    assert(sDirectXCommon->GetDevice());
    material_.Initialize();
    CreateMesh();
}

void Particle::Draw(const std::vector<InstancingBufferData>& bufferData, const ViewProjection& viewProjection, const DirectionalLight& directionalLight, const Vector4& color, const uint32_t textureHadle) {
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

    TextureManager::GetInstance()->SetGraphicsRootDescriptorTable(sCommandList, static_cast<UINT>(RootParameter::kTexture), textureHadle);

    sCommandList->DrawIndexedInstanced(static_cast<UINT>(indices_.size()), static_cast<UINT>(bufferData.size()), 0, 0, 0);
}
