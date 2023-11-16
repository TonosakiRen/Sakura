#include "Bloom.h"

#include <cassert>

#include "DirectXCommon.h"
#include "CommandContext.h"
#include "Helper.h"

#include "externals/DirectXTex/d3dx12.h"

using namespace DirectX;
using namespace Microsoft::WRL;

DirectXCommon* Bloom::sDirectXCommon = nullptr;

void Bloom::Initialize(ColorBuffer* originalTexture)
{
    sDirectXCommon = DirectXCommon::GetInstance();
    // メッシュ生成
    CreateMesh();

    originalTexture_ = originalTexture;

    luminanceTexture_.Create(
        originalTexture_->GetWidth(),
        originalTexture_->GetHeight(),
        originalTexture->GetFormat());

    gaussianBlurs_[0].Initialize(&luminanceTexture_);
    for (uint32_t i = 1; i < kMaxLevel; ++i) {
        gaussianBlurs_[i].Initialize(&gaussianBlurs_[i - 1].GetResult());
    }

    {
        CD3DX12_DESCRIPTOR_RANGE ranges[kMaxLevel]{};
        for (uint32_t i = 0; i < kMaxLevel; ++i) {
            ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, i);
        }

        CD3DX12_ROOT_PARAMETER rootParameters[kMaxLevel + 1]{};
        rootParameters[0].InitAsConstants(2, 0);
        for (uint32_t i = 0; i < kMaxLevel; ++i) {
            rootParameters[i + 1].InitAsDescriptorTable(1, &ranges[i]);
        }

        CD3DX12_STATIC_SAMPLER_DESC staticSamplerDesc(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.pParameters = rootParameters;
        rootSignatureDesc.NumParameters = _countof(rootParameters);
        rootSignatureDesc.pStaticSamplers = &staticSamplerDesc;
        rootSignatureDesc.NumStaticSamplers = 1;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        rootSignature_.Create(rootSignatureDesc);
    }

    {
        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
      {
       "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {
       "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };


        DXGI_FORMAT format = originalTexture_->GetFormat();

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
        psoDesc.pRootSignature = rootSignature_;

        ComPtr<IDxcBlob> vs = sDirectXCommon->CompileShader(L"PostEffectVS.hlsl", L"vs_6_0");
        ComPtr<IDxcBlob> ps = sDirectXCommon->CompileShader(L"LuminanceExtractionrPS.hlsl", L"ps_6_0");
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());

        psoDesc.InputLayout.pInputElementDescs = inputLayout;
        psoDesc.InputLayout.NumElements = _countof(inputLayout);

        psoDesc.BlendState = Helper::BlendDisable;
        psoDesc.RasterizerState = Helper::RasterizerNoCull;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = format;
        psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.SampleDesc.Count = 1;

        luminacePipelineState_.Create(psoDesc);

        ps = sDirectXCommon->CompileShader(L"BloomPS.hlsl", L"ps_6_0");
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());

        psoDesc.BlendState = Helper::BlendAdditive;
        additivePipelineState_.Create(psoDesc);
    }

}

void Bloom::Render(CommandContext& commandContext, uint32_t level)
{
    assert(level <= kMaxLevel);
    if (threshold_ == 0.0f) {
        threshold_ = 0.00001f;
    }
    if (threshold_ == 1.0f) {
        threshold_ = 1.00001f;
    }

    commandContext.TransitionResource(*originalTexture_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandContext.TransitionResource(luminanceTexture_, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandContext.SetRenderTarget(luminanceTexture_.GetRTV());
    commandContext.ClearColor(luminanceTexture_);
    commandContext.SetViewportAndScissorRect(0, 0, luminanceTexture_.GetWidth(), luminanceTexture_.GetHeight());

    commandContext.SetRootSignature(rootSignature_);

    commandContext.SetVertexBuffer(0, vbView_);
    commandContext.SetIndexBuffer(ibView_);


    commandContext.SetPipelineState(luminacePipelineState_);
    commandContext.SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandContext.SetConstants(0, float(threshold_), float(knee_));
    commandContext.SetDescriptorTable(1, originalTexture_->GetSRV());
    commandContext.DrawIndexed(static_cast<UINT>(indices_.size()), 0, 0);


    for (uint32_t i = 0; i < level; ++i) {
        gaussianBlurs_[i].Render(commandContext);
    }

    commandContext.TransitionResource(*originalTexture_, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandContext.SetRenderTarget(originalTexture_->GetRTV());
    commandContext.SetViewportAndScissorRect(0, 0, originalTexture_->GetWidth(), originalTexture_->GetHeight());

    commandContext.SetRootSignature(rootSignature_);
    commandContext.SetPipelineState(additivePipelineState_);
    commandContext.SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandContext.SetConstants(0, float(intensity_));
    for (uint32_t i = 0; i < level; ++i) {
        commandContext.SetDescriptorTable(i + 1, gaussianBlurs_[i].GetResult().GetSRV());
    }
    commandContext.DrawIndexed(static_cast<UINT>(indices_.size()), 0, 0);

    commandContext.TransitionResource(*originalTexture_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}


void Bloom::CreateMesh()
{
    HRESULT result = S_FALSE;

    vertices_.resize(4);

    //左下
    vertices_[0].pos = { -1.0f,-1.0f,0.0f,1.0f };
    vertices_[0].uv = { 0.0f,1.0f };
    //左上
    vertices_[1].pos = { -1.0f,1.0f,0.0f,1.0f };
    vertices_[1].uv = { 0.0f,0.0f };
    //右上
    vertices_[2].pos = { 1.0f,1.0f,0.0f,1.0f };
    vertices_[2].uv = { 1.0f,0.0f };
    //右下
    vertices_[3].pos = { 1.0f,-1.0f,0.0f,1.0f };
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

    // 頂点バッファへのデータ転送
    VertexData* vertMap = nullptr;
    result = vertBuff_->Map(0, nullptr, (void**)&vertMap);
    if (SUCCEEDED(result)) {
        std::copy(vertices_.begin(), vertices_.end(), vertMap);
        vertBuff_->Unmap(0, nullptr);
    }

    // 頂点バッファビューの作成
    vbView_.BufferLocation = vertBuff_->GetGPUVirtualAddress();
    vbView_.SizeInBytes = sizeVB;
    vbView_.StrideInBytes = sizeof(vertices_[0]);


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

    // インデックスバッファへのデータ転送
    uint16_t* indexMap = nullptr;
    result = indexBuff_->Map(0, nullptr, (void**)&indexMap);
    if (SUCCEEDED(result)) {
        std::copy(indices_.begin(), indices_.end(), indexMap);

        indexBuff_->Unmap(0, nullptr);
    }

    // インデックスバッファビューの作成
    ibView_.BufferLocation = indexBuff_->GetGPUVirtualAddress();
    ibView_.Format = DXGI_FORMAT_R16_UINT;
    ibView_.SizeInBytes = sizeIB;

}
